#!/bin/bash

NOW=$(date +%Y%m%d%H%M)

# Backup Configuration
BACKUP_HOME="openmct@mfs.cbrp3.c-base.org:/usr/home/openmct/backup"
CURRENT_LINK="$BACKUP_HOME/current"
SNAPSHOT_DIR="$BACKUP_HOME/$NOW"

# Folder to backup
BACKUP_SOURCE_DIR="/tmp/influxdbbackup/"

sudo rm /tmp/influxdbbackup/cbeam.backup -rf
sudo docker exec influxdb influxd backup -database cbeam /backup/cbeam.backup
sudo chown -R alarm:alarm /tmp/influxdbbackup/

rsync -avz -e "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" --progress \
	$BACKUP_SOURCE_DIR $SNAPSHOT_DIR

sudo rm /tmp/influxdbbackup/cbeam.backup -rf
