# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the Linux kbox implementation
#

kbox-y += kbox_main.o kbox_notifier.o kbox_console.o kbox_collect.o kbox_netlink.o kbox_cdev.o kbox_monitor.o kbox_ram_image.o kbox_ram_op.o kbox_dump.o

obj-$(CONFIG_KBOX)      += kbox.o
