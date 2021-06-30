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
    const timeout = process.env.TIMEOUT * 1000;
    (async () => {
        await firestore
            .collection("heartbeats")
            .get()
            .then((docs) => {
                docs.forEach((doc) => {
                    const heartbeat = doc.get("heartbeat");
                    const now = Firestore.Timestamp.now();
                    console.debug(`Processing ${doc.id} - comparing ${heartbeat.toMillis()} with ${now.toMillis()}...`);

                    const difference = now.toMillis() - heartbeat.toMillis();
                    if (difference > timeout) {
                        console.log(`Notifying Slack because we haven't seen a heartbeat for ${difference} ms since ${heartbeat.toDate()}, timeout is ${timeout} ms...`);
                        (async () => {
                            await webhook.send({
                                icon_emoji: ':chicken:',
                                text: `No heartbeat for *${doc.id}* for *${difference} ms* since *${heartbeat.toDate()}* (timeout is *${timeout} ms*).`,
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
