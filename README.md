This is the source code for the [Lilac Loop VCV Rack plugin website](https://grough.github.io/lilac-loop-vcv/).

Run `make latest` to update the pre-release download page. This requires the following environment variables to be set:

```txt
OWNER
REPO
WORKFLOW_ID
GITHUB_API_TOKEN
OUTPUT_DIR
```

Optionally, load environment variables from a password manager:

```sh
op run --env-file="./make_latest.env" -- make latest
```
