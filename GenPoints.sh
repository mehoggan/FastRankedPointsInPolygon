#!/bin/bash

PARAMS=""
while (( "$#" )); do
  case "$1" in
    -n|--number_of_points)
      NPOINTS=$2
      shift 2
      ;;
    -o|--output_file)
      OUTPUT_FILE=$2
      shift 2
      ;;
    --) # end argument parsing
      shift
      break
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
    *) # preserve positional arguments
      PARAMS="$PARAMS $1"
      shift
      ;;
  esac
done
# set positional arguments in their proper place
eval set -- "$PARAMS"

if [ "${NPOINTS}" == "" ];
then
	echo "Failed to get number of points."
	echo "Usage ./GenPoints.sh -n [Number of Points] -o [Output File]"
	exit 1
fi
if [ "${OUTPUT_FILE}" == "" ] || [ ! -e "${OUTPUT_FILE}" ];
then
	echo "Failed to find ${OUTPUT_FILE}."
	echo "Usage ./GenPoints.sh -n [Number of Points] -o [Output File]"
	exit 1
fi

echo -n "" > "${OUTPUT_FILE}"
for i in $(seq 0 $((NPOINTS - 1)));
do
	echo "Generating point $((i + 1))..."
  a=-10000
  b=+10000
  echo -n "${i} $((1 + RANDOM % NPOINTS)) " \
          >> "${OUTPUT_FILE}"
  echo -n "$((a + RANDOM % (b - a))).$((RANDOM%999)) " >> "${OUTPUT_FILE}"
  echo -n "$((a + RANDOM % (b - a))).$((RANDOM%999)) " >> "${OUTPUT_FILE}"

  echo "" >> "${OUTPUT_FILE}"
done
