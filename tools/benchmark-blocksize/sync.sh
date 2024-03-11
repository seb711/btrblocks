#!/bin/bash

# Function to read CSV file and sync URIs
sync_uris() {
  input_file="$1"
  output_file="$2"
  factor="$3"
  index=1

  while IFS=',' read -r name uri yaml; do
    schemaname=$(basename "$yaml")

    echo $uri

    if [[ ! -f "./csvtobtrdata/yaml/$name/$schemaname" ]]; then
      mkdir ./csvtobtrdata/yaml/$name -p
      aws s3 cp $yaml ./csvtobtrdata/yaml/$name/ --no-sign
    fi

    btr_dir="./csvtobtrdata/btrblocks/$name/"
    mkdir -p "$btr_dir" || rm -rf "$btr_dir"/*
    bin_dir="./csvtobtrdata/btrblocks_bin/$name/"
    echo "aws s3 sync $uri $bin_dir"
    if [[ ! -d $bin_dir ]]; then
      aws s3 sync $uri $bin_dir --no-sign
    fi

    yaml_file="./csvtobtrdata/yaml/$name/$schemaname"
    ./csvtobtr --btr $btr_dir --binary $bin_dir --create_btr true --yaml $yaml_file

    echo "$factor, $schemaname, $(./decompression-speed --btr $btr_dir --reps 100 --binary $bin_dir --yaml $yaml_file --verify)" >> $output_file

    ((index++))

  done < "$input_file"
}

# install things
sudo apt-get install libssl-dev libcurl4-openssl-dev aws-cli -y

# build the benchmark thing
output_file="results.csv"
rm $output_file
mkdir tmpbuild
cd tmpbuild

dataset="../datasetsuris.csv"
# Check if uris.csv exists
if [[ ! -f $dataset ]]; then
  echo "datasetsurismall.csv file not found."
  exit 1
fi

for chunksize in {12..13}
do
  echo "BUILDING FOR CHUNKSIZE $chunksize"
  cmake ../../.. -DCHUNKSIZE=$chunksize -DCMAKE_BUILD_TYPE=Release
  make -j6 csvtobtr
  make -j6 decompression-speed
  # Sync URIs from the CSV file
  # sync_uris "parquet_s3_files.csv" > "./decompression-output-$replacement.txt"
  sync_uris $dataset $output_file $chunksize
done

