# METIS Pipeline
METIS pipe line is the data reduction software for ELT early science instrument of Mid-IR imager and spectrograph.

## Manual Installation

See [`README.md`](../metisp) for manual installation of the pipeline.

## Installation via UV

Clone the installation script and run it

```
git clone https://github.com/eiseleb47/metis-meta-package.git
cd metis-meta-package/
./bootstrap.sh
```

select `y` to install the pipeline and optionally select to run some simulations for simulated data. Then run use the UV run command to execute pipeline commands...

```
uv run --env-file .env pyesorex --recipes
uv run --env-file .env edps -P 4444 -s
```

## Installation via Docker

Follow the installation instructions for [Docker Engine](https://docs.docker.com/engine/install/)


Clone the git repo

```
git clone https://github.com/AstarVienna/METIS_Pipeline.git
```

Build the image

```
cd METIS_Pipeline/toolbox/
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

