/** Because WebUSB is only accessible via a secure context, this file is intended to be copied & pasted into the
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
