const IncomingWebhook = require('@slack/webhook').IncomingWebhook;
const webhook = new IncomingWebhook(process.env.SLACK_URL);

const Firestore = require('@google-cloud/firestore');
const firestore = new Firestore({
    projectId: process.env.PROJECT_ID
});

/**
 * Cloud Function entry point, HTTP trigger.
 * @param {Object} req The HTTP request.
 * @param {object} res The HTTP response.
 */
exports.checkHeartbeat = (req, res) => {
    const now = Firestore.Timestamp.now();
    (async () => {
        await firestore
            .collection("heartbeats")
            .get()
            .then((docs) => {
                docs.forEach((doc) => {
                    const heartbeat = doc.get("heartbeat");
                    console.debug(`Processing ${doc.id} - comparing ${heartbeat.toMillis()} with ${now.toMillis()}...`);

                    if (heartbeat.toMillis() < now.toMillis() - process.env.TIMEOUT * 1000) {
                        console.log("Notifying Slack because we haven't seen a heartbeat for some time...");
                        (async () => {
                            await webhook.send({
                                icon_emoji: ':chicken:',
                                text: `No heartbeat for *${doc.id}*!`,
                            });
                        })();
                    }
                });
            })
            .then(() => {
                res.send("OK");
            })
            .catch((error) => {
                console.error(error);
                res.status(500).send(error);
            });
    })();
};
