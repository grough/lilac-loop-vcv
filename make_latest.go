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

var owner string = os.Getenv("OWNER")
var repo string = os.Getenv("REPO")
var token string = os.Getenv("GITHUB_API_TOKEN")
var workflowId string = os.Getenv("WORKFLOW_ID")
var outputDir string = os.Getenv("OUTPUT_DIR")
var apiBase string = fmt.Sprintf("https://api.github.com/repos/%s/%s", owner, repo)

func main() {
	run, err := GetLatestRun(workflowId)
	if err != nil {
		fmt.Println("Error getting run ID:", err)
		os.Exit(1)
	}
	run.Artifacts, err = getArtifacts(run.Id)
	if err != nil {
		fmt.Println("Error getting artifacts:", err)
		os.Exit(1)
	}
	for _, artifact := range run.Artifacts {
		err := downloadArtifact(token, artifact, outputDir)
		if err != nil {
			fmt.Println("Error downloading artifact:", err)
			os.Exit(1)
		}
	}
	html, err := makeHtml(run)
	if err != nil {
		fmt.Println("Error formatting HTML:", err)
		os.Exit(1)
	}
	htmlPath := filepath.Join(outputDir, "index.html")
	if err := ioutil.WriteFile(htmlPath, []byte(html), 0644); err != nil {
		fmt.Println("Error writing HTML to file:", err)
		os.Exit(1)
	}
}

func GetLatestRun(workflowId string) (Run, error) {
	resp, err := get(fmt.Sprintf("%s/actions/workflows/%s/runs?page=1&per_page=1", apiBase, workflowId))
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
		fmt.Println("Error parsing response body:", err)
		return Run{}, err
	}

	if len(data.Runs) == 0 {
		return Run{}, fmt.Errorf("No workflow runs found")
	}

	return data.Runs[0], nil
}

func getArtifacts(runId int) ([]Artifact, error) {
	resp, err := get(fmt.Sprintf("%s/actions/runs/%d/artifacts", apiBase, runId))
	if err != nil {
		fmt.Println("Request error:", err)
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

func downloadArtifact(token string, artifact Artifact, downloadDir string) error {
	err := os.MkdirAll(downloadDir, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to create download directory: %v %v", downloadDir, err)
	}
	fileName := artifact.Name + ".zip"
	filePath := filepath.Join(downloadDir, fileName)
	out, err := os.Create(filePath)
	if err != nil {
		return fmt.Errorf("failed to create output file: %v", err)
	}
	defer out.Close()

	resp, err := get(artifact.ArchiveDownloadUrl)
	if err != nil {
		fmt.Println("Request error:", err)
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to download artifact (status code %d)", resp.StatusCode)
	}

	_, err = io.Copy(out, resp.Body)
	if err != nil {
		return fmt.Errorf("failed to write output file: %v", err)
	}

	return nil
}

func makeHtml(run Run) (string, error) {
	tmpl, err := template.New("downloads").Parse(`<style>body { font-size: 20px; }</style>
<h1>Lilac Loop Development Builds</h1>
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
</small>`)
	if err != nil {
		return "", err
	}
	var buf bytes.Buffer
	err = tmpl.Execute(&buf, run)
	if err != nil {
		return "", err
	}
	return buf.String(), nil
}

func get(url string) (*http.Response, error) {
	var req *http.Request
	var err error
	req, err = http.NewRequest("GET", url, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Accept", "application/vnd.github+json")
	req.Header.Set("Authorization", fmt.Sprintf("Bearer %s", token))
	req.Header.Set("X-GitHub-Api-Version", "2022-11-28")
	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	return resp, nil
}
