# Publish build artifacts on the web

Run `make latest` to generate a web page listing build artifacts for download. The following environment variables are rquired:

```
OWNER
REPO
WORKFLOW_ID
GITHUB_API_TOKEN
OUTPUT_DIR
```

Optionally, load environment variables from a password manager:
```
op run --env-file="./make_latest.env" -- make latest
```