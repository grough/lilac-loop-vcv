# Publish build artifacts on the web

- Download artifact files from latest workflow run
- Generate HTML linking to each file
- Commit HTML and files to website
- Publish website

```sh
OWNER="grough" REPO="lilac-loop-vcv" GITHUB_API_TOKEN="…" DOWNLOAD_DIR="$(pwd)/latest" make artifacts > latest/index.html
```