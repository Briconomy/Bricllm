# Bricllm Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g -D_GNU_SOURCE
LDFLAGS = -lm
TARGET = bricllm
SRCDIR = src
INCDIR = include

# Source directories
COREDIR = $(SRCDIR)/core
UTILSDIR = $(SRCDIR)/utils
ROUTESDIR = $(SRCDIR)/routes
DATADIR = $(SRCDIR)/data

# Source files
CORE_SOURCES = $(COREDIR)/chat_engine.c $(COREDIR)/pattern_matcher.c
UTILS_SOURCES = $(UTILSDIR)/logger.c $(UTILSDIR)/pattern_cache.c $(UTILSDIR)/conversation_context.c
ROUTES_SOURCES = $(ROUTESDIR)/tenant_routes.c
DATA_SOURCES = $(DATADIR)/route_system.c

MAIN_SOURCE = main.c

# All sources
SOURCES = $(CORE_SOURCES) $(UTILS_SOURCES) $(ROUTES_SOURCES) $(DATA_SOURCES) $(MAIN_SOURCE)

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Built $(TARGET) successfully!"

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build artifacts"

# Run the application
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Test build
test: $(TARGET)
	@echo "Running basic tests..."
	./$(TARGET) < tests/test_input.txt
	@echo "Tests completed"

# Install (placeholder for future use)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall (placeholder for future use)
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build the application (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  run      - Build and run the application"
	@echo "  debug    - Build with debug symbols"
	@echo "  test     - Build and run tests"
	@echo "  install  - Install to system"
	@echo "  uninstall- Remove from system"
	@echo "  help     - Show this help message"

# Dependency tracking
-include $(OBJECTS:.o=.d)

# Generate dependency files
%.d: %.c
	@$(CC) $(CFLAGS) -I$(INCDIR) -MM $< > $@

.PHONY: all clean run debug test install uninstall help