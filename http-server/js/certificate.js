import crypto from "crypto"

// certificate + privae + public key creation
export default function certificate() {
    const { publicKey, privateKey } = crypto.generateKeyPairSync("rsa", {
        modulusLength: 2048,
    });

    return {
        publicKey: publicKey.export({ type: "pkcs1", format: "pem" }),
        privateKey: privateKey.export({ type: "pkcs1", format: "pem" }),
    };
}

