.. zephyr:board:: mt8395_genio_1200_evk

Overview
********

The `MediaTek Genio 1200 EVK`_ is an evaluation board for the Genio 1200
(MT8395) application processor. The MT8395 combines four Cortex-A78 cores at
2.2 GHz with four Cortex-A55 cores at 2.0 GHz, an Arm Mali-G57 MC5 GPU, two
Tensilica VP6 vision DSPs and a Tensilica HiFi 4 audio DSP.

.. _MediaTek Genio 1200 EVK:
   https://genio.mediatek.com/doc/iot-yocto/latest/hw/g1200-evk.html

MT8395 is the SoC powering the Genio 1200 platform. Linux drives it as an
MT8195 (its devicetree lists ``mediatek,mt8195``), so this board uses the
``mt8195`` SoC and MT8395 appears only in the board name.

Zephyr supports the ``mt8395_genio_1200_evk/mt8195/adsp`` board target, which
runs on the HiFi 4 audio DSP as Sound Open Firmware loaded by Linux. See
`Audio DSP`_.

Hardware
********

The audio DSP uses the following memory. The DRAM regions must match the ones
the Linux board devicetree (``mt8395-genio-1200-evk.dts``) reserves for the DSP:

=========  ==========  =======  =======================================================
Node       Address     Size     Use
=========  ==========  =======  =======================================================
``sram0``  0x40000000  256 KB   DSP SRAM, holding the firmware heap
``dram0``  0x60000000  13.5 MB  Firmware, data and IPC windows (``adsp_mem_reserved``)
``dram1``  0x60d80000  2.5 MB   Host DMA buffers (``adsp_dma_mem_reserved``)
=========  ==========  =======  =======================================================

The 8 MB above ``dram1``, from 0x61000000, holds the host AFE buffers
(``snd_dma_mem``) and is not used by the DSP. The generic
``mt8195/mt8195/adsp`` target places ``dram1`` at 0x60e80000 instead, which does
not match this board's reserved memory, so firmware built for that target does
not work here.

The DSP reaches the off-chip audio hardware through the audio front end (AFE)
memory interfaces, whose devicetree nodes are in :file:`afe-mt8195.dtsi`.

Supported Features
==================

.. zephyr:board-supported-hw::

Audio DSP
*********

The ``mt8395_genio_1200_evk/mt8195/adsp`` target is meant for the
`Sound Open Firmware`_ (SOF) application. Linux boots the DSP through its
MT8195 SOF driver, which loads the signed image from
``/lib/firmware/mediatek/sof/sof-mt8195.ri`` and the topology
``sof-mt8195.tplg`` from ``/lib/firmware/mediatek/sof-tplg``, and exchanges
IPC with it over the ADSP mailboxes.

.. _Sound Open Firmware:
   https://thesofproject.github.io/latest/

The `Genio audio DSP porting guide`_ describes the target side: the kernel
options needed for SOF logging with ``sof-logger``, and how to build and
install topologies.

.. _Genio audio DSP porting guide:
   https://genio.mediatek.com/doc/iot-yocto/latest/sw/yocto/porting-guide/audio/audio-porting-adsp.html

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
``/lib/firmware/mediatek/sof/sof-mt8195.ri``, the name the kernel expects on
this board, and reboot it. Linux loads the DSP firmware during boot.

Debugging
=========

No Zephyr flash or debug runner is provided: the image is loaded by the Linux
SOF driver rather than by a host-side tool, so ``west flash`` and ``west debug``
do not apply to this board. The Zephyr console is written to a winstream buffer
in DSP memory (:kconfig:option:`CONFIG_WINSTREAM_CONSOLE`).

References
**********

Genio 1200 product page:
    https://genio.mediatek.com/genio-1200

Genio 1200 EVK hardware documentation:
    https://genio.mediatek.com/doc/iot-yocto/latest/hw/g1200-evk.html

Genio audio DSP porting guide:
    https://genio.mediatek.com/doc/iot-yocto/latest/sw/yocto/porting-guide/audio/audio-porting-adsp.html

IoT Yocto documentation:
    https://genio.mediatek.com/doc/iot-yocto/latest/
