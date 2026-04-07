import net from "net";
import crypto from "crypto"

const port = 8000;

const clientRandom = crypto.randomBytes(32);
let serverRandom = null;
const premaster = crypto.randomBytes(48);
let sessionKey = null;

let publicKey = null;

const client = net.createConnection({ port }, () => {
    console.log("Connected to server");

    // Simulate handshake
    client.write(JSON.stringify({
        type: "CLIENT_HELLO",
        clientRandom: clientRandom.toString("hex")
    }));

});



client.on("data", (data) => {
    const msg = JSON.parse(data.toString());
    console.log("Received from server:", msg);
    // client.end()

    if (msg.type === "SERVER_HELLO") {
        serverRandom = Buffer.from(msg.serverRandom, "hex");
        publicKey = msg.publicKey;

        const encryptedPremaster = crypto.publicEncrypt(
            {
                key: publicKey,
            },
            premaster
        );

        // exchange encrypted premaster
        client.write(
            JSON.stringify({
                type: "CLIENT_KEY_EXCHANGE",
                encryptedPremaster: encryptedPremaster.toString("base64"),
            })
        );

        sessionKey =
            crypto.createHash("sha256")
                .update(premaster)
                .update(clientRandom)
                .update(serverRandom)
                .digest();

        console.log("Session Key (client):", sessionKey.toString("hex"));
    }
});

client.on("end", () => {
    console.log("Disconnected from server");
});
