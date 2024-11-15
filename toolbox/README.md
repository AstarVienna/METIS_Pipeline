# METIS Pipeline
METIS pipe line is the data reduction software for ELT early science instrument of Mid-IR imager and spectrograph.

## Manual Installation

See the `README.md` in the `metisp` directory for manual installation of the pipeline.

## Installation via Docker

Follow the installation instructions for [Docker Engine](https://docs.docker.com/engine/install/)


Clone the git repo

```
git clone https://github.com/AstarVienna/METIS_Pipeline.git
```

Build the image

```
docker build -t metispipeline .
```

Run the image

```
docker run -ti --net=host metispipeline
```


List all images and containers

```
docker image list
docker container list
```

Delete image or container

```
docker container rm <nameofcontainer>
docker image rm metispipeline
```

## Running the pipeline

While inside the docker image

```
pyesorex --recipes 
pyesorex metis_det_lingain "${SOF_DIR}/metis_det_lingain.lm.sof"
edps -s
edps -w metis.metis_lm_img_wkf -i $SOF_DATA -c
edps -w metis.metis_lm_img_wkf -i $SOF_DATA
```

