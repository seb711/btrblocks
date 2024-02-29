#!/bin/bash

# Function to read CSV file and sync URIs
sync_uris() {
  input_file="$1"
  output_file="$2"
  factor="$3"
  index=1

  while IFS=',' read -r _ uri yaml; do
    echo "aws s3 cp $uri ./$index"
    echo "aws s3 cp $yaml ./$index"

    filename=$(basename "$uri")
    schemaname=$(basename "$yaml")


    if [[ ! -f "./csvtobtrdata/raw_data/$index/$filename" ]]; then
      mkdir ./csvtobtrdata/raw_data/$index -p
      aws s3 cp $uri ./csvtobtrdata/raw_data/$index/ --no-sign
      sed -i "s/\"//g" ./csvtobtrdata/raw_data/$index/$filename
    fi

    if [[ ! -f "./csvtobtrdata/raw_data/$index/$schemaname" ]]; then
      aws s3 cp $yaml ./csvtobtrdata/raw_data/$index/ --no-sign
    fi

    bin_dir="./csvtobtrdata/btrblocks_bin/$index/"
    btr_dir="./csvtobtrdata/btrblocks/$index/"
    mkdir -p "$bin_dir" || rm -rf "$bin_dir"/*
    mkdir -p "$btr_dir" || rm -rf "$btr_dir"/*
    ./csvtobtr --btr $btr_dir --binary $bin_dir --yaml ./csvtobtrdata/raw_data/$index/$schemaname --csv ./csvtobtrdata/raw_data/$index/$filename --create_binary true --create_btr true
    echo "$factor, $filename, $(./decompression-speed --btr $btr_dir --reps 5)" >> $output_file

    ((index++))

  done < "$input_file"
}

if ! command -v aws &> /dev/null; then
  echo "AWS CLI not found. Please install it and configure."
  exit 1
fi

# Check if uris.csv exists
if [[ ! -f "./datasets.csv" ]]; then
  echo "datasets.csv file not found."
  exit 1
fi

# install things
sudo apt-get install libssl-dev libcurl4-openssl-dev -y

# build the benchmark thing
output_file="results.csv"
mkdir tmpbuild
cd tmpbuild
for chunksize in {12..16}
do
  echo "BUILDING FOR CHUNKSIZE $chunksize"
  cmake ../../.. -DCHUNKSIZE=$chunksize -DPARTSIZE=16 -DCMAKE_BUILD_TYPE=Release
  make -j4 csvtobtr
  make -j4 decompression-speed

  rm $output_file

  # Sync URIs from the CSV file
  # sync_uris "parquet_s3_files.csv" > "./decompression-output-$replacement.txt"
  sync_uris "../datasets.csv" $output_file $chunksize
done