.. zephyr:board:: mt8365_genio_350_evk

Overview
********

The `MediaTek Genio 350 EVK`_ is an evaluation board for the Genio 350 (MT8365)
application processor. The MT8365 combines four Cortex-A53 cores at 2.0 GHz
with an Arm Mali-G52 GPU, a Tensilica VP6 vision DSP and a Tensilica HiFi 4
audio DSP.

.. _MediaTek Genio 350 EVK:
   https://genio.mediatek.com/doc/iot-yocto/latest/hw/i350-evk.html

Zephyr supports the ``mt8365_genio_350_evk/mt8365/adsp`` board target, which
runs on the HiFi 4 audio DSP as Sound Open Firmware loaded by Linux. See
`Audio DSP`_.

Hardware
********

The audio DSP uses the following memory. All of it except ``sram1`` lies in
system DRAM that the Linux board devicetree (``mt8365-evk.dts``) reserves for
the DSP:

==========  ==========  =======  ==================================================
Node        Address     Size     Use
==========  ==========  =======  ==================================================
``sram1``   0x1e000000  416 KB   DSP SRAM, holding the firmware heap
``sram0``   0x40020000  256 KB   DRAM used as DSP SRAM (``adsp_sram``)
``dram0``   0x60000000  13.5 MB  Firmware, data and IPC windows (``adsp_reserved``)
``dram1``   0x60d80000  2.5 MB   Host DMA buffers (``adsp_reserved_dma``)
==========  ==========  =======  ==================================================

The DSP reaches the off-chip audio hardware through the audio front end (AFE)
memory interfaces, whose devicetree nodes are in :file:`afe-mt8365.dts`.

Supported Features
==================

.. zephyr:board-supported-hw::

Audio DSP
*********

The ``mt8365_genio_350_evk/mt8365/adsp`` target is meant for the
`Sound Open Firmware`_ (SOF) application. Linux boots the DSP through its SOF
driver, which loads the signed image from
``/lib/firmware/mediatek/sof/sof-mt8365.ri`` and the topology
``sof-mt8365.tplg`` from ``/lib/firmware/mediatek/sof-tplg``, and exchanges
IPC with it over the ADSP mailboxes.

.. _Sound Open Firmware:
   https://thesofproject.github.io/latest/

Programming and Debugging
*************************

Building
========

Build and sign the SOF application for this target as described in the SOF
build guide. The image must carry an extended manifest, which SOF adds when
``CONFIG_RIMAGE_SIGNING_SCHEMA`` is set; the SOF board configuration for this
target sets it.

Loading
=======

Install the signed ``zephyr.ri`` on the target as
``/lib/firmware/mediatek/sof/sof-mt8365.ri`` and reboot it. Linux loads the DSP
firmware during boot.

Debugging
=========

No Zephyr flash or debug runner is provided: the image is loaded by the Linux
SOF driver rather than by a host-side tool, so ``west flash`` and ``west debug``
do not apply to this board. The Zephyr console is written to a winstream buffer
in DSP memory (:kconfig:option:`CONFIG_WINSTREAM_CONSOLE`).

References
**********

Genio 350 product page:
    https://genio.mediatek.com/genio-350

Genio 350 EVK hardware documentation:
    https://genio.mediatek.com/doc/iot-yocto/latest/hw/i350-evk.html

IoT Yocto documentation:
    https://genio.mediatek.com/doc/iot-yocto/latest/
