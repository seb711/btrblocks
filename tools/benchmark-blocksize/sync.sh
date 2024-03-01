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


    if [[ ! -f "./csvtobtrdata/raw_data_small/$index/$filename" ]]; then
      mkdir ./csvtobtrdata/raw_data_small/$index -p
      aws s3 cp $uri ./csvtobtrdata/raw_data_small/$index/ --no-sign
      sed -i "s/\"//g" ./csvtobtrdata/raw_data_small/$index/$filename
    fi

    if [[ ! -f "./csvtobtrdata/raw_data_small/$index/$schemaname" ]]; then
      aws s3 cp $yaml ./csvtobtrdata/raw_data_small/$index/ --no-sign
    fi

    btr_dir="./csvtobtrdata/btrblocks_small/$index/"
    mkdir -p "$btr_dir" || rm -rf "$btr_dir"/*
    bin_dir="./csvtobtrdata/btrblocks_bin_small/$index/"
    if [[ ! -d $bin_dir ]]; then
      mkdir -p "$bin_dir"
      ./csvtobtr --btr $btr_dir --binary $bin_dir --yaml ./csvtobtrdata/raw_data_small/$index/$schemaname --csv ./csvtobtrdata/raw_data_small/$index/$filename --create_binary true --create_btr true
    else
      ./csvtobtr --btr $btr_dir --binary $bin_dir --yaml ./csvtobtrdata/raw_data_small/$index/$schemaname --csv ./csvtobtrdata/raw_data_small/$index/$filename --create_btr true
    fi

    echo "$factor, $filename, $(./decompression-speed --btr $btr_dir --reps 5)" >> $output_file

    ((index++))

  done < "$input_file"
}

if ! command -v aws &> /dev/null; then
  echo "AWS CLI not found. Please install it and configure."
  exit 1
fi

# install things
sudo apt-get install libssl-dev libcurl4-openssl-dev -y

# build the benchmark thing
output_file="resultssmall.csv"
mkdir tmpbuild
cd tmpbuild

dataset="../datasetssmall.csv"
# Check if uris.csv exists
if [[ ! -f $dataset ]]; then
  echo "datasets.csv file not found."
  exit 1
fi

for chunksize in {14..16}
do
  echo "BUILDING FOR CHUNKSIZE $chunksize"
  cmake ../../.. -DCHUNKSIZE=$chunksize -DPARTSIZE=16 -DCMAKE_BUILD_TYPE=Release
  make -j4 csvtobtr
  make -j4 decompression-speed

  rm $output_file

  # Sync URIs from the CSV file
  # sync_uris "parquet_s3_files.csv" > "./decompression-output-$replacement.txt"
  sync_uris $dataset $output_file $chunksize
done

