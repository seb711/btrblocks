# Compiler settings
CXX := g++
CXXFLAGS := -O3 -march=native
LDFLAGS :=

# Target names
TARGET_PLAIN := program_plain
TARGET_NON_TEMPORAL := program_non_temporal

# Arguments
N := 536870912
RLE := 64
ITERATIONS := 5
REPS := 5
FACTOR := 1

# Default target
all: $(TARGET_PLAIN) $(TARGET_NON_TEMPORAL)

# Plain version
$(TARGET_PLAIN): rle_lower_bound.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Non-temporal version
$(TARGET_NON_TEMPORAL): rle_lower_bound.cpp
	$(CXX) $(CXXFLAGS) -DUSE_NON_TEMPORAL $< -o $@ $(LDFLAGS)

# Run targets
run_plain: $(TARGET_PLAIN)
	sudo ./$(TARGET_PLAIN) $(N) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) > data/plain.csv

run_non_temporal: $(TARGET_NON_TEMPORAL)
	sudo ./$(TARGET_NON_TEMPORAL) $(N) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) > data/nontemporal.csv

# Run with dynamic N based on L3 cache size
run_l3_n: $(TARGET_PLAIN) $(TARGET_NON_TEMPORAL)
	@$(eval N_DYNAMIC := $(shell echo $$((`cat /sys/devices/system/cpu/cpu0/cache/index3/size | grep -o '[0-9]\+'` * 1024/ 4))))
	@$(eval FACTOR := $(shell echo "scale=2; $(N) / $(N_DYNAMIC)" | bc))
	@echo "Running with N_DYNAMIC=$(N_DYNAMIC), FACTOR=$(FACTOR)"
	@sudo ./$(TARGET_PLAIN) $(N_DYNAMIC) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) > data/l3.csv
	@sudo ./$(TARGET_NON_TEMPORAL) $(N_DYNAMIC) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) > data/l3nt.csv

run_all:
	@sudo $(MAKE) run_plain
	@sudo $(MAKE) run_non_temporal
	@sudo $(MAKE) run_l3_n

clean:
	rm -f $(TARGET_PLAIN) $(TARGET_NON_TEMPORAL)
	sudo rm data/*

.PHONY: all clean run_plain run_non_temporal
