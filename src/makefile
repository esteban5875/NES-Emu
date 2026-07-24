# =============================================================================
#  NES-Emu — top-level orchestrator
#
#  Coordinates the three MVC components that live next to this file:
#    - nes-emu-model       C   emulator core        (built via its own makefile)
#    - nes-emu-controller  C++ websocket backend    (bridges model <-> view)
#    - nes-emu-view        Vue frontend             (Vite dev/build)
#
#  The controller is still a stub; targets that touch it degrade gracefully
#  (with a notice) until it has a real makefile + server binary, at which
#  point `build` and `dev` pick it up automatically.
#
#  Common usage:
#    make setup     install frontend dependencies
#    make dev       build everything, then run backend + Vue dev server
#    make build     build model + controller + view (production)
#    make test      build and run the model's C test harness
#    make clean     remove build artifacts
# =============================================================================

# Anchor all paths to this makefile's directory so it works from any CWD
# (e.g. `make -C src dev` or `make -f src/makefile dev`).
ROOT       := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))

MODEL_DIR      := $(ROOT)/nes-emu-model
CONTROLLER_DIR := $(ROOT)/nes-emu-controller
VIEW_DIR       := $(ROOT)/nes-emu-view

# Expected controller server binary once the controller is implemented.
CONTROLLER_BIN := $(CONTROLLER_DIR)/bin/controller

NPM := npm --prefix $(VIEW_DIR)

.DEFAULT_GOAL := help

# ---- Help -------------------------------------------------------------------
.PHONY: help
help:
	@echo "NES-Emu — available targets:"
	@echo "  setup            install the Vue app's dependencies"
	@echo "  build            build model + controller + view"
	@echo "  dev              run the backend + Vue dev server (Ctrl-C stops both)"
	@echo "  test             build and run the model's C test harness"
	@echo "  preview          serve the production Vue build locally"
	@echo "  clean            remove build artifacts"
	@echo "  distclean        clean + remove node_modules"
	@echo ""
	@echo "  per-component: build-model build-controller build-view"
	@echo "                 clean-model clean-controller clean-view"

# ---- Setup ------------------------------------------------------------------
.PHONY: setup
setup: $(VIEW_DIR)/node_modules

# Install only when node_modules is missing.
$(VIEW_DIR)/node_modules: $(VIEW_DIR)/package.json
	$(NPM) install
	@touch $@

# ---- Build ------------------------------------------------------------------
.PHONY: build build-model build-controller build-view
build: build-model build-controller build-view

build-model:
	$(MAKE) -C $(MODEL_DIR)

# Delegate to the controller's makefile once it has content; skip while empty.
build-controller:
	@if [ -s $(CONTROLLER_DIR)/makefile ]; then \
		$(MAKE) -C $(CONTROLLER_DIR); \
	else \
		echo ">> controller: empty stub — nothing to build yet (skipping)"; \
	fi

build-view: $(VIEW_DIR)/node_modules
	$(NPM) run build

# ---- Run --------------------------------------------------------------------
# Starts the controller backend (if built) and the Vue dev server together.
# `trap 'kill 0'` tears down the background backend when the dev server exits.
.PHONY: dev
dev: build-model build-controller $(VIEW_DIR)/node_modules
	@echo "── Starting NES-Emu (Ctrl-C to stop) ──"
	@trap 'kill 0' EXIT INT TERM; \
	if [ -x "$(CONTROLLER_BIN)" ]; then \
		echo ">> starting controller backend: $(CONTROLLER_BIN)"; \
		"$(CONTROLLER_BIN)" & \
	else \
		echo ">> controller backend not built yet — running frontend only"; \
	fi; \
	echo ">> starting Vue dev server"; \
	$(NPM) run dev

# Run the model's standalone test program.
.PHONY: test
test:
	$(MAKE) -C $(MODEL_DIR) run

# Serve the production build (run `make build-view` first).
.PHONY: preview
preview: $(VIEW_DIR)/node_modules
	$(NPM) run preview

# ---- Clean ------------------------------------------------------------------
.PHONY: clean clean-model clean-controller clean-view distclean
clean: clean-model clean-controller clean-view

clean-model:
	$(MAKE) -C $(MODEL_DIR) clean

clean-controller:
	@if [ -s $(CONTROLLER_DIR)/makefile ]; then \
		$(MAKE) -C $(CONTROLLER_DIR) clean; \
	else \
		echo ">> controller: empty stub — nothing to clean (skipping)"; \
	fi

clean-view:
	rm -rf $(VIEW_DIR)/dist

distclean: clean
	rm -rf $(VIEW_DIR)/node_modules
