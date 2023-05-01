.PHONY: latest # Download all artifacts from the latest build
.PHONY: clean # Remove all artifacts

latest:
	@go run make_latest.go
	
clean:
	@rm -rf latest/*
