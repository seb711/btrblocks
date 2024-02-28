#!/bin/bash

# Function to read CSV file and sync URIs
sync_uris() {
  input_file="$1"
  index=1

  while IFS=',' read -r _ uri yaml; do
    echo "aws s3 cp $uri ./$index"
    echo "aws s3 cp $yaml ./$index"

    filename=$(basename "$uri")
    schemaname=$(basename "$yaml")


    if [[ ! -f "./csvtobtrdata/raw_data/$index/$filename" ]]; then
      mkdir ./csvtobtrdata/raw_data/$index -p
      aws s3 cp $uri ./csvtobtrdata/raw_data/$index/ --no-sign
    fi

    if [[ ! -f "./csvtobtrdata/raw_data/$index/$schemaname" ]]; then
      aws s3 cp $yaml ./csvtobtrdata/raw_data/$index/ --no-sign
    fi

    bin_dir="./csvtobtrdata/btrblocks_bin/$index/"
    btr_dir="./csvtobtrdata/btrblocks/$index/"
    mkdir -p "bin_dir" || rm -rf "bin_dir"/*
    mkdir -p "btr_dir" || rm -rf "btr_dir"/*
    ./csvtobtr --btr $btr_dir --binary $bin_dir --yaml ./csvtobtrdata/$index/$schemaname --csv ./csvtobtrdata/$index/$filename --create_binary true --create_btr true
    echo "$filename, $(./decompression-speed --btr $btr_dir --reps 5)" >> results.csv

    ((index++))

  done < "$input_file"
}

if ! command -v aws &> /dev/null; then
  echo "AWS CLI not found. Please install it and configure."
  exit 1
fi

# Check if uris.csv exists
if [[ ! -f "../tools/benchmark-blocksize/data-fetch/datasetssmall.csv" ]]; then
  echo "datasets.csv file not found."
  exit 1
fi

# install things
sudo apt-get install libthrift-dev libbrotli-dev libboost-all-dev libsnappy-dev libssl-dev libcurl4-openssl-dev -y

# build the benchmark thing
make -j4 csvtobtr
make -j4 decompression-speed

rm "results.txt"

# Sync URIs from the CSV file
# sync_uris "parquet_s3_files.csv" > "./decompression-output-$replacement.txt"
sync_uris "../tools/benchmark-blocksize/data-fetch/datasetssmall.csv"