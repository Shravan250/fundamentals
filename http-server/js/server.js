import crypto from "crypto";
import net from "net";
import certificate from "./certificate.js";

const port = 8000;
const serverRandom = crypto.randomBytes(32);
let clientRandom = null;
let premaster = null;
let sessionKey = null;

const { publicKey, privateKey } = certificate()

const server = net.createServer((socket) => {
    console.log("Client connected");

    socket.on("data", (data) => {
        const msg = JSON.parse(data.toString());
        console.log("Received:", msg);

        if (msg.type === "CLIENT_HELLO") {
            clientRandom = Buffer.from(msg.clientRandom, "hex");

            socket.write(
                JSON.stringify({
                    type: "SERVER_HELLO",
                    serverRandom: serverRandom.toString("hex"),
                    publicKey,
                })
            );
        }
        else if (msg.type === "CLIENT_KEY_EXCHANGE") {
            premaster = crypto.privateDecrypt(
                {
                    key: privateKey,
                },
                Buffer.from(msg.encryptedPremaster, "base64")
            );

            sessionKey = crypto
                .createHash("sha256")
                .update(premaster)
                .update(clientRandom)
                .update(serverRandom)
                .digest();

            console.log("Session Key (server):", sessionKey.toString("hex"));
        }
    });

    socket.on("end", () => {
        console.log("Client disconnected");
    });
});

server.listen(port, () => {
    console.log(`Server listening on port ${port}`);
});




