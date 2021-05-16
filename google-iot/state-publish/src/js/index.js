const IncomingWebhook = require('@slack/webhook').IncomingWebhook;
const webhook = new IncomingWebhook(process.env.SLACK_URL);

/**
 * Cloud Function entry point, Cloud Pub/Sub trigger.
 * Extracts the metrics data from payload and insert to BigQuery
 * @param {Object} event The event payload.
 * @param {object} context The event metadata.
 */
exports.statePublish = (event, context) => {
    const pubsubMessage = event.data;
    const deviceId = event.attributes.deviceId;
    const objStr = Buffer.from(pubsubMessage, 'base64').toString();
    const message = JSON.parse(objStr);
    console.debug(message);

    (async () => {
        await webhook.send({
            icon_emoji: ':chicken:',
            text: `${describeEvent(message.event)} \`\`\`${JSON.stringify(message)}\`\`\``,
        });
    })();
};

function describeEvent(event) {
    if (event.init) {
        return "Initialized";
    } else if (event.hasOwnProperty("emergencyStop")) {
        return "Emergency stop activated!";
    } else if (event.hasOwnProperty("state")) {
        return `The door is now *${describeState(event.state)}*.`;
    } else {
        return `Unknown event: \`${JSON.stringify(event)}\`.`;
    }
}

function describeState(state) {
    switch (state) {
        case 2:
            return "open";
        case 1:
            return "opening";
        case -1:
            return "closing";
        case -2:
            return "closed";
        default:
            return `unknown (${state})`;
    }
}
