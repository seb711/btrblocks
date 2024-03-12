# Compiler settings
CXX := g++
CXXFLAGS := -O3 -march=native
LDFLAGS :=

# Target names
TARGET_PLAIN := program_autovec
TARGET_NON_TEMPORAL := program_non_temporal
TARGET_AVX := program_avx2

# Arguments
N := 536870912
RLE := 64
ITERATIONS := 5
REPS := 5
FACTOR := 1

DATA := dataxps
TMP_FILE := tmp_output.csv

# Default target
all: $(TARGET_PLAIN) $(TARGET_NON_TEMPORAL)

# autovec version
$(TARGET_PLAIN): rle_lower_bound.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Non-temporal version
$(TARGET_NON_TEMPORAL): rle_lower_bound.cpp
	$(CXX) $(CXXFLAGS) -DUSE_NON_TEMPORAL $< -o $@ $(LDFLAGS)

# avx version
$(TARGET_AVX): rle_lower_bound.cpp
	$(CXX) $(CXXFLAGS) -DUSE_AVX2 $< -o $@ $(LDFLAGS)

# Run targets
run_plain: $(TARGET_PLAIN)
	sudo ./$(TARGET_PLAIN) $(N) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) "autovec" >> $(DATA)/$(TMP_FILE)


run_non_temporal: $(TARGET_NON_TEMPORAL)
	sudo ./$(TARGET_NON_TEMPORAL) $(N) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) "non_temporal" >> $(DATA)/$(TMP_FILE)

run_avx: $(TARGET_AVX)
	sudo ./$(TARGET_AVX) $(N) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) "avx" >> $(DATA)/$(TMP_FILE)

# Run with dynamic N based on L3 cache size
# constant 6 is the number of threads because the l3 cache is somehow split up per cache idk have to take a look into it
run_l3_n: $(TARGET_PLAIN) $(TARGET_NON_TEMPORAL) $(TARGET_AVX)
	@$(eval N_DYNAMIC := $(shell echo $$((`cat /sys/devices/system/cpu/cpu0/cache/index3/size | grep -o '[0-9]\+'` * 1024 / 4 / 6)))) 
	@$(eval FACTOR := $(shell echo "scale=2; $(N) / $(N_DYNAMIC)" | bc))
	@echo "Running with N_DYNAMIC=$(N_DYNAMIC), FACTOR=$(FACTOR)"
	@sudo ./$(TARGET_PLAIN) $(N_DYNAMIC) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) "l3_autovec" >> $(DATA)/$(TMP_FILE)
	@sudo ./$(TARGET_NON_TEMPORAL) $(N_DYNAMIC) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) "l3_non_temporal" >> $(DATA)/$(TMP_FILE)
	@sudo ./$(TARGET_AVX) $(N_DYNAMIC) $(RLE) $(ITERATIONS) $(REPS) $(FACTOR) "l3_avx" >> $(DATA)/$(TMP_FILE)

run_all:
	mkdir -p $(DATA)
	@sudo $(MAKE) run_plain
	@sudo $(MAKE) run_non_temporal
	@sudo $(MAKE) run_avx
	@sudo $(MAKE) run_l3_n

clean:
	rm -f $(TARGET_PLAIN) $(TARGET_NON_TEMPORAL) $(TARGET_AVX)
	sudo rm $(DATA)/$(TMP_FILE)

.PHONY: all clean run_plain run_non_temporal
