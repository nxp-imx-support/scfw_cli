/*
 * Copyright 2018 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/
#ifndef CONTROLS_H
#define CONTROLS_H

/* Array of control strings */
const char *sc_ctl_str[] = {
"SC_C_TEMP",
"SC_C_TEMP_HI",
"SC_C_TEMP_LOW",
"SC_C_PXL_LINK_MST1_ADDR",
"SC_C_PXL_LINK_MST2_ADDR",
"SC_C_PXL_LINK_MST_ENB",
"SC_C_PXL_LINK_MST1_ENB",
"SC_C_PXL_LINK_MST2_ENB",
"SC_C_PXL_LINK_SLV1_ADDR",
"SC_C_PXL_LINK_SLV2_ADDR",
"SC_C_PXL_LINK_MST_VLD",
"SC_C_PXL_LINK_MST1_VLD",
"SC_C_PXL_LINK_MST2_VLD",
"SC_C_SINGLE_MODE",
"SC_C_ID",
"SC_C_PXL_CLK_POLARITY",
"SC_C_LINESTATE",
"SC_C_PCIE_G_RST",
"SC_C_PCIE_BUTTON_RST",
"SC_C_PCIE_PERST",
"SC_C_PHY_RESET",
"SC_C_PXL_LINK_RATE_CORRECTION",
"SC_C_PANIC",
"SC_C_PRIORITY_GROUP",
"SC_C_TXCLK",
"SC_C_CLKDIV",
"SC_C_DISABLE_50",
"SC_C_DISABLE_125",
"SC_C_SEL_125",
"SC_C_MODE",
"SC_C_SYNC_CTRL0",
"SC_C_KACHUNK_CNT",
"SC_C_KACHUNK_SEL",
"SC_C_SYNC_CTRL1",
"SC_C_DPI_RESET",
"SC_C_MIPI_RESET",
"SC_C_DUAL_MODE",
"SC_C_VOLTAGE",
"SC_C_PXL_LINK_SEL",
"SC_C_OFS_SEL",
"SC_C_OFS_AUDIO",
"SC_C_OFS_PERIPH",
"SC_C_OFS_IRQ",
"SC_C_RST0",
"SC_C_RST1",
"SC_C_SEL0",
"SC_C_CALIB0",
"SC_C_CALIB1",
"SC_C_CALIB2",
"SC_C_IPG_DEBUG",
"SC_C_IPG_DOZE",
"SC_C_IPG_WAIT",
"SC_C_IPG_STOP",
"SC_C_IPG_STOP_MODE",
"SC_C_IPG_STOP_ACK",
"SC_C_LAST"
};
#define sc_ctl_str_len  (sizeof(sc_ctl_str)/sizeof(sc_ctl_str[0]))

#endif
