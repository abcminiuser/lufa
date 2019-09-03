/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2019  Benjamin Riggs (https://github.com/riggs)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/


/**
 * Because WebUSB is only accessible via a secure context, this file is intended to be copied & pasted into the
 * developer console on a page loaded via https.
 */

const WEBUSB_IO_EPSIZE = 8;

let device = await navigator.usb.requestDevice({filters: [ {vendorId: 0x03eb, productId: 0x206f} ]});

await device.open();

device.configuration;   // This should exist. If it is null, run `await device.selectConfiguration(1);`

await device.claimInterface(0);

async function receive(device) {
    let endpoint_id;
    for ( const endpoint of device.configuration.interfaces[0].alternate.endpoints ) {
        if ( endpoint.direction === 'in' && endpoint.type === 'interrupt' ) {
            endpoint_id = endpoint.endpointNumber;
            break;
        }
    }

    const result = await device.transferIn(endpoint_id, WEBUSB_IO_EPSIZE);
    if ( result.status !== "ok" ) {
        throw new Error(`Transfer in failed with status "${result.status}"`);
    }
    const data_view = result.data;
    return Array.from(new Uint8Array(data_view.buffer))
}

async function poll(device) {
    console.log(await receive(device));
    setTimeout(() => poll(device), 0);
}

async function send(...data) {
    let endpoint_id;
    for ( const endpoint of device.configuration.interfaces[0].alternate.endpoints ) {
        if ( endpoint.direction === 'out' && endpoint.type === 'interrupt' ) {
            endpoint_id = endpoint.endpointNumber;
            break;
        }
    }

    const buffer = Uint8Array.from(data.slice(0, WEBUSB_IO_EPSIZE)).buffer;
    const result = await device.transferOut(endpoint_id, buffer);
    if ( result.status !== "ok" ) {
        throw new Error(`Transfer out failed with status "${result.status}"`);
    }
    return result.bytesWritten;
}

poll(device);

await send(1, 2, 3, 4, 5);
