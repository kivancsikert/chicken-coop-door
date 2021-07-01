const {BigQuery} = require('@google-cloud/bigquery');
const bigquery = new BigQuery();

const { Firestore } = require('@google-cloud/firestore');
const firestore = new Firestore({
  projectId: process.env.PROJECT_ID
});

/**
 * Cloud Function entry point, Cloud Pub/Sub trigger.
 * Extracts the metrics data from payload and insert to BigQuery
 * @param {Object} event The event payload.
 * @param {object} context The event metadata.
 */
exports.galagonyaPublish = (event, context) => {
  const pubsubMessage = event.data;
  const deviceId = event.attributes.deviceId;
  const objStr = Buffer.from(pubsubMessage, 'base64').toString();
  const msgObj = JSON.parse(objStr);
  const now = new Date();
  const timestamp = BigQuery.timestamp(now);
  let rows = [{
    device_id: deviceId,
    time: timestamp,
    light: msgObj.light,
    gate: msgObj.gate,
    motorPosition: msgObj.motorPosition,
    openSwitch: msgObj.openSwitch,
    closedSwitch: msgObj.closedSwitch
  }];
  insertRowsAsStream(rows);

  (async () => {
    await firestore
      .doc(`heartbeats/${deviceId}`)
      .set({
        heartbeat: now
      }, {
          merge: true
      });
  })();
};

function insertRowsAsStream (rows) {
  bigquery
    .dataset(process.env.DATASET)
    .table(process.env.TABLE)
    .insert(rows);
}
