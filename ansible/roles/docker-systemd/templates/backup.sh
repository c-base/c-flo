#!/bin/bash

NOW=$(date +%Y%m%d%H%M)
YESTERDAY=$(date -v -1d +%Y%m%d)

# Backup Configuration
BACKUP_HOME="mfs.cbrp3.c-base.org:/backups/influxdb"
CURRENT_LINK="$BACKUP_HOME/current"
SNAPSHOT_DIR="/tmp/snapshots"

# Folder to backup
BACKUP_SOURCE_DIR="/var/influxdb"

# Init the folder structure
mkdir -p $SNAPSHOT_DIR &> /dev/null

tar cfv "$SNAPSHOT_DIR/$NOW.tar" $BACKUP_SOURCE_DIR

rsync -avz -e "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" --progress \
	$SNAPSHOT_DIR/$NOW.tar $BACKUP_HOME 

# && ln -snf $(ls -1d $SNAPSHOT_DIR/* | tail -n1) $CURRENT_LINK
