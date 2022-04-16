import { Client } from './external/tmi.js';

function getBTTVChannelEmotes(channelName) {
    
}
function getBTTVEmoteURL(emoteID){return `https://cdn.betterttv.net/emote/${emoteID}/3x`;}



const token = localStorage.access_token;
if(token)
{
    runChatbox(token)
}

const runChatbox = (token) =>
{
    const client = new Client({
        options: { debug: true },
        identity: {
            username: 'kf48fc5oafct9wqb1jf3lrsfurujq2',
            password: 'oauth:'+token
        },
        channels: [ 'Sharples' ]
    });

    client.connect();

    client.on('message', (channel, tags, message, self) => {
        // "Alca: Hello, World!"
        console.log(`${tags['display-name']}: ${message}`);
    });
}
