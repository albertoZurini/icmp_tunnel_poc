PANDOC = pandoc
INPUT  = slides.md
OUTPUT = output/index.html

# revealjs-url must NOT end with / so paths become .../dist/... not ...//dist/...
FLAGS = -t revealjs -s \
        -V revealjs-url=https://unpkg.com/reveal.js@5.2.1 \
        --metadata-file=preamble.yaml

# Default: embed CSS/JS so the deck works offline (requires network at build time)
FLAGS_EMBED = $(FLAGS) --embed-resources --standalone
# Use this if build has no network: output will load Reveal from CDN when viewed in browser
FLAGS_NO_EMBED = $(FLAGS) --standalone

all: $(OUTPUT)

$(OUTPUT): $(INPUT) preamble.yaml
	@mkdir -p output
	$(PANDOC) $(INPUT) -o $@ $(FLAGS_EMBED)

clean:
	rm -rf output/

serve: all
	@echo "Starting simple HTTP server at http://localhost:8000"
	python3 -m http.server --directory output 8000

# Export Reveal.js deck to PDF (requires Node/npx for decktape; starts a short-lived server)
PDF_OUT = output/slides.pdf
pdf: all
	@echo "Starting temporary server for decktape..."
	@(cd output && python3 -m http.server 9999 &); \
	SERVER_PID=$$!; \
	sleep 2; \
	npx --yes decktape reveal --fragments http://127.0.0.1:9999/index.html $(PDF_OUT); \
	EXIT=$$?; kill $$SERVER_PID 2>/dev/null || true; exit $$EXIT
	@echo "PDF written to $(PDF_OUT)"

.PHONY: all clean serve pdf