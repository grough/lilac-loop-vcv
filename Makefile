.PHONY: latest # Download all artifacts from the latest build
.PHONY: clean # Remove all artifacts

latest:
	@cd make_latest && go run main.go
	
clean:
	rm -rf latest/*
