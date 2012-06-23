DOCUMENT_NAME = LUFA-next
OUTPUT_DIR    = Output
PACKAGE_DIR   = Packages

# Build the LaTeX document.
all: outputdir pdf
	
# Remove output directory.
clean:
	rm -rf $(OUTPUT_DIR)



# Create LaTeX output directory.
outputdir:
	$(shell mkdir $(OUTPUT_DIR) 2>/dev/null)
	
# Generate PDF output from LaTeX input files.
pdf:
	pdflatex -interaction=errorstopmode --include-directory=$(PACKAGE_DIR) --output-directory=$(OUTPUT_DIR) $(DOCUMENT_NAME)
	bibtex -quiet $(OUTPUT_DIR)/$(DOCUMENT_NAME)
	pdflatex -quiet --include-directory=$(PACKAGE_DIR) --output-directory=$(OUTPUT_DIR) $(DOCUMENT_NAME)
	pdflatex -quiet --include-directory=$(PACKAGE_DIR) --output-directory=$(OUTPUT_DIR) $(DOCUMENT_NAME)
	pdflatex -quiet --include-directory=$(PACKAGE_DIR) --output-directory=$(OUTPUT_DIR) $(DOCUMENT_NAME)