.PHONY: artifacts # Download all artifacts from the latest build
.PHONY: clean # Remove all artifacts

artifacts:
	@cd make_artifacts && go run main.go
	
clean:
	rm -rf latest/*

	