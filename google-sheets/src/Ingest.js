const spreadsheet = SpreadsheetApp.openById("1PAZUGVEqr0QuLZsgKPX3FqNu_ouogwcfPW63xPMQ23Q");
const sheet = spreadsheet.getSheetByName("DATA");

const doPost = (request) => {
    console.log(request);
    const payload = JSON.parse(request.postData.contents);
    const data = JSON.parse(String.fromCharCode.apply(null, Utilities.base64Decode(payload.message.data)));
    sheet.appendRow([
        new Date(payload.message.publishTime),
        payload.message.attributes.deviceId,
        payload.message.attributes.deviceNumId,
        data.light,
        data.gate,
        data.motorPosition,
        data.openSwitch,
        data.closedSwitch
    ]);
    return ContentService.createTextOutput();
};
