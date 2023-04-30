package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"html/template"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"path/filepath"
)

type Run struct {
	Id        int    `json:"id"`
	Url       string `json:"html_url"`
	Sha       string `json:"head_sha"`
	CreatedAt string `json:"created_at"`
	Artifacts []Artifact
}

type Artifact struct {
	Id                 int    `json:"id"`
	Name               string `json:"name"`
	ArchiveDownloadUrl string `json:"archive_download_url"`
}

func main() {
	owner := os.Getenv("OWNER")
	repo := os.Getenv("REPO")
	token := os.Getenv("GITHUB_API_TOKEN")
	downloadDir := os.Getenv("DOWNLOAD_DIR")
	run, err := GetRun(owner, repo, token)
	if err != nil {
		fmt.Println("Error getting run ID:", err)
		os.Exit(1)
	}
	artifacts, err := GetArtifacts(owner, repo, token, run.Id)
	if err != nil {
		fmt.Println("Error getting artifacts:", err)
		os.Exit(1)
	}
	run.Artifacts = artifacts
	var paths []string
	for _, artifact := range artifacts {
		path, err := DownloadArtifact(token, artifact, downloadDir)
		if err != nil {
			fmt.Println("Error downloading artifact:", err)
			os.Exit(1)
		}
		paths = append(paths, path)
	}
	html, err := Format(run)
	if err != nil {
		fmt.Println("Error formatting HTML:", err)
		os.Exit(1)
	}
	fmt.Println(html)
}

// Return the ID of the most recent workflow run
func GetRun(owner string, repo string, token string) (Run, error) {
	url := fmt.Sprintf("https://api.github.com/repos/%s/%s/actions/runs?page=1&per_page=1", owner, repo)

	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		fmt.Println("Error creating request:", err)
		return Run{}, err
	}
	req.Header.Set("Accept", "application/vnd.github+json")
	req.Header.Set("Authorization", fmt.Sprintf("Bearer %s", token))
	req.Header.Set("X-GitHub-Api-Version", "2022-11-28")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		fmt.Println("Error making request:", err)
		return Run{}, err
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Println("Error reading response body:", err)
		return Run{}, err
	}

	var data struct {
		Runs []Run `json:"workflow_runs"`
	}

	if err := json.Unmarshal(body, &data); err != nil {
		fmt.Println("Error unmarshaling response body:", err)
		return Run{}, err
	}

	if len(data.Runs) == 0 {
		return Run{}, fmt.Errorf("No workflow runs found")
	}

	return data.Runs[0], nil
}

// Get build artifacts for a given workflow run ID
func GetArtifacts(owner string, repo string, token string, runId int) ([]Artifact, error) {
	artifactsUrl := fmt.Sprintf("https://api.github.com/repos/%s/%s/actions/runs/%d/artifacts", owner, repo, int(runId))
	req, err := http.NewRequest("GET", artifactsUrl, nil)
	if err != nil {
		fmt.Println("Error creating request:", err)
		return []Artifact{}, err
	}
	req.Header.Set("Accept", "application/vnd.github+json")
	req.Header.Set("Authorization", fmt.Sprintf("Bearer %s", token))
	req.Header.Set("X-GitHub-Api-Version", "2022-11-28")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		fmt.Println("Error making request:", err)
		return []Artifact{}, err
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Println("Error reading response body:", err)
		return []Artifact{}, err
	}

	var data struct {
		Artifacts []Artifact `json:"artifacts"`
	}

	if err := json.Unmarshal(body, &data); err != nil {
		fmt.Println("Error unmarshaling response body:", err)
		return []Artifact{}, err
	}

	return data.Artifacts, nil
}

// Download an artifact
func DownloadArtifact(token string, artifact Artifact, downloadDir string) (string, error) {
	// Create the target directory if it doesn't exist yet
	err := os.MkdirAll(downloadDir, os.ModePerm)
	if err != nil {
		return "", fmt.Errorf("failed to create download directory: %v %v", downloadDir, err)

	}

	// Create the output file path based on the artifact name and the target directory
	fileName := artifact.Name + ".zip"
	filePath := filepath.Join(downloadDir, fileName)

	// Open the output file for writing
	out, err := os.Create(filePath)
	if err != nil {
		return "", fmt.Errorf("failed to create output file: %v", err)
	}
	defer out.Close()

	// Create a new HTTP request to download the artifact
	req, err := http.NewRequest("GET", artifact.ArchiveDownloadUrl, nil)
	if err != nil {
		return "", fmt.Errorf("failed to create HTTP request: %v", err)
	}
	req.Header.Set("Authorization", "Bearer "+token)

	// Send the HTTP request and get the response
	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return "", fmt.Errorf("failed to send HTTP request: %v", err)
	}
	defer resp.Body.Close()

	// Check that the response status code is 200 OK
	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("failed to download artifact (status code %d)", resp.StatusCode)
	}

	// Copy the response body to the output file
	_, err = io.Copy(out, resp.Body)
	if err != nil {
		return "", fmt.Errorf("failed to write output file: %v", err)
	}

	return fileName, nil
}

func Format(run Run) (string, error) {
	tmpl, err := template.New("downloads").Parse(`
<style>body { font-size: 20px; }</style>
<h1>Lilac Loop Pre-Release Builds</h1>
<p>Download for your platform:</p>
<ul>
{{range .Artifacts}}
  <li>
    <a href="{{.Name}}.zip">{{.Name}}.zip</a>
  </li>
{{end}}
</ul>
<hr />
<small>
	<p>Updated at {{.CreatedAt}}</p>
	<p>Generated by <a href="{{.Url}}">{{.Url}}</a></p>
	<p>From commit <a href="https://github.com/grough/lilac-loop-vcv/commit/{{.Sha}}">{{.Sha}}</a></p>
</small>
`)
	if err != nil {
		return "", err
	}

	// Execute the template and store the output in a buffer
	var buf bytes.Buffer
	err = tmpl.Execute(&buf, run)
	if err != nil {
		return "", err
	}

	// Convert the buffer to a string and print it
	return buf.String(), nil
}
