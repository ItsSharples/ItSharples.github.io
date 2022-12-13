
class twitchChannel {
    constructor() { };

    // channel_id, badges, emotes, customEmotes
    get channelID() { return this.m_channel_id; };
    get badges() { return this.m_badges; };
    get emotes() { return this.m_emotes; };
    get customEmotes() { return this.m_customEmotes; };
    get config() { return this.m_config; };

    async loadChannelData(config) {
        console.log(config);
        var channel_id = await getChannelID(config);

        var badges = [];
        badges = badges.concat(await getGlobalBadges(config, channel_id));
        
        var channelBadges = []
        channelBadges = channelBadges.concat(await getChannelBadges(config, channel_id));
        channelBadges.forEach(channelBadge => 
            {
                var index = badges.findIndex(badge => badge.set_id == channelBadge.set_id);
                // Couldn't find the value
                if (index == -1){
                    badges.push(channelBadge)
                }
                else {
                    badges[index] = channelBadge
                }
            }
        );
        console.log(channelBadges)


        var emotes = [];
        var customEmotes = [];

        //console.log(await get7TVChannelEmotes(config, "50030128"));
        //console.log(await getFFZChannelEmotes(config, "50030128"));

        if (!!config.enableEmotes) {
            if (!!config.emotesTwitch) {
                emotes = emotes.concat(await getGlobalEmotes(config, channel_id));
                emotes = emotes.concat(await getChannelEmotes(config, channel_id));
            }
            if (!!config.customEmotes) {
                if (!!config.emotesBTTV) {
                    customEmotes = customEmotes.concat(await getBTTVChannelEmotes(config, channel_id));
                }
                if (!!config.emotes7TV) {
                    customEmotes = customEmotes.concat(await get7TVChannelEmotes(config, "50030128"));
                }
                if (!!config.emotesFFZ) {
                    customEmotes = customEmotes.concat(await getFFZChannelEmotes(config, "50030128"));
                }
            }
        }

        this.m_config = config;
        this.m_channel_id = channel_id;
        this.m_badges = badges;
        this.m_emotes = emotes;
        this.m_customEmotes = customEmotes;

        console.log("Loaded Channel Data:", customEmotes);
    }
}
const scaleLookup = {
    "1": "18w", "2": "36w", "3": "72w", "4": "72w",
    "1x": "18w", "2x": "36w", "3x": "72w", "4x": "72w"
};
let client_id = "kf48fc5oafct9wqb1jf3lrsfurujq2";
var client;
var state;
state = new twitchChannel();
if (location.hash) {
    setupServer(location.hash.substring(1))
    setInterval(() => { window.scrollBy({ 'top': window.innerHeight, 'behavior': 'instant' }); }, 1000);
}
else {
    if (document.getElementById("config")) {
        try {
            var b64Config;
            if (!!sessionStorage.config) {
                b64Config = sessionStorage.config;
            }
            else {
                b64Config = "eyJjaGFubmVsTmFtZSI6InNoYXJwbGVzIiwiZW5hYmxlRW1vdGVzIjoib24iLCJlbW90ZU9ubHkiOiJ0cnVlIiwiZW1vdGVzVHdpdGNoIjoib24iLCJlbW90ZXNCVFRWIjoib24iLCJmb250RmFtaWx5Ijoic3lzdGVtLXVpIiwiZm9udFNpemUiOiI1MCIsImJhZGdlU2l6ZSI6IjUwIiwiZW1vdGVTaXplIjoiNTAiLCJhY2Nlc3NUb2tlbiI6IjJ2dm10b2E1eDlqbmZkN3hhMmI1dmszZGdvcnRtYSJ9";
            }
            setupServer(b64Config);
            document.addEventListener('update-config', () => {
                const config = Base64ToJSON(sessionStorage.config);
                state.loadChannelData(config);
                updateStyle(config);
            });
        }
        catch (err) {
            console.log(err);
        }
    }
    else {
        sendToConfigurator();
    }
}



function setupServer(hash) {
    let config = setupWebpage(hash);
    state.loadChannelData(config);
    listenToMessages(config);
}
function updateStyle(config) {
    const style = document.getElementById("chatbox").style;

    style.setProperty("--font-family", config.fontFamily);

    style.setProperty("--font-height", config.fontSize + "px");
    style.setProperty("--font-height-half", (config.fontSize / 2) + "px");

    style.setProperty("--background-colour", config.backgroundColor ?? "transparent");
}
function setupWebpage(hash) {
    // Convert Hash to the JSON
    const config = Base64ToJSON(hash);

    config.customEmotes = (!!config.emotesBTTV) || (!!config.emotes7TV) || (!!config.emotesFFZ);

    updateStyle(config);

    client?.disconnect();
    client = new tmi.Client({
        options: {
            skipUpdatingEmotesets: true,
            debug: false,
        },
        identity: {
            username: client_id,
            password: 'oauth:' + config.accessToken
        },
        channels: [config.channelName],
        
    });

    client.connect();
    return config;
}

function sendToConfigurator() {
    window.location.href = "/chatbox";
}
function JSONtoBase64(json) { return btoa(JSON.stringify(json)); }
function Base64ToJSON(b64Data) { return JSON.parse(atob(b64Data)); }

async function getChannelID(config) {
    return fetch(`https://api.twitch.tv/helix/users?login=${config.channelName}`, {
        headers: {
            "Authorization": `Bearer ${config.accessToken}`,
            "Client-Id": client_id
        }
    })
        .then(response => response.json())
        .then(data => data.data[0].id);
}

async function getGlobalBadges(config, id) {
    return fetch("https://api.twitch.tv/helix/chat/badges/global", {
        headers: {
            "Authorization": `Bearer ${config.accessToken}`,
            "Client-Id": client_id
        }
    })
        .then(response => response.json())
        .then(data => data.data);
}

async function getChannelBadges(config, id) {
    return fetch(`https://api.twitch.tv/helix/chat/badges?broadcaster_id=${id}`, {
        headers: {
            "Authorization": `Bearer ${config.accessToken}`,
            "Client-Id": client_id
        }
    })
        .then(response => response.json())
        .then(data => data.data);
}

async function getGlobalEmotes(config, id) {
    return fetch(`https://api.twitch.tv/helix/chat/emotes/global`, {
        headers: {
            "Authorization": `Bearer ${config.accessToken}`,
            "Client-Id": client_id
        }
    })
        .then(response => response.json())
        .then(data => data.data);
}

async function getChannelEmotes(config, id) {
    return fetch(`https://api.twitch.tv/helix/chat/emotes?broadcaster_id=${id}`, {
        headers: {
            "Authorization": `Bearer ${config.accessToken}`,
            "Client-Id": client_id
        }
    })
        .then(response => response.json())
        .then(data => data.data);
}

async function getBTTVChannelEmotes(config, id) {
    return fetch(`https://api.betterttv.net/3/cached/users/twitch/${id}`)
        .then(response => response.json())
        .then(data => data.channelEmotes.concat(data.sharedEmotes))
        .then(emotes => {
            let output = [];
            for (index in emotes) {
                let emote = emotes[index];
                let newEmote = {};
                newEmote["name"] = emote["code"];
                newEmote["url"] = `https://cdn.betterttv.net/emote/${emote.id}/3x`;
                let imgSet = [];
                for (scale of ["1x", "2x", "3x"]) {
                    imgSet.push(`https://cdn.betterttv.net/emote/${emote.id}/${scale} ${scaleLookup[scale]}`);
                }
                newEmote["imgSet"] = imgSet;
                output.push(newEmote);
            }
            return output;
        })
}

async function get7TVChannelEmotes(config, id) {
    return fetch(`https://api.7tv.app/v2/users/${id}/emotes`)
        .then(response => response.json())
        .then(data => Object.values(data).map(emote => {
            let index = 0;
            let imgSet = [];
            for (width of emote.width) {
                let tuple = emote.urls[index];
                let url = tuple[1];
                imgSet.push(`${url} ${width}w`);
                index++;
            }
            emote.imgSet = imgSet;
            return emote
        }
        ))
}

async function getFFZChannelEmotes(config, id) {
    return fetch(`https://api.frankerfacez.com/v1/room/id/${id}`)
        .then(response => response.json())
        .then(data => data.sets)
        .then(sets => Object.values(sets)
            .map(set => Object.values(set.emoticons)
                .map(emote => {
                    //console.log(emote)
                    let value = {};
                    value.name = emote.name;
                    let urls = Object.entries(emote.urls).map(
                        ([scale, url], i) => `https:${url} ${scaleLookup[scale]}`
                    )
                    emote.imgSet = urls;
                    return emote
                })
            )).then(emotes => emotes.flat())

}

async function loadPronouns(username) {
    return fetch(`https://pronouns.alejo.io/api/users/${username}`)
    .then(response => response.json())
    .then(data => data.data)

}

function listenToMessages(config) {
    function setIdEquals(name) { return (object, index, array) => object.set_id == name; }

    function IdEquals(name) { return (object, index, array) => object.id == name; }

    client.on('raw_message', (...args) => {
        //console.log(args);

        switch(args["1"].command){
            case "PRIVMSG": {
                return;
            }
            case "PONG": {
                console.log("PING")
                break;
            }
            case "PING": {
                console.log("PING")
                break;
            }
            case "USERNOTICE": {
                console.log("Announcment? ModCheck")
                console.log(args)

                console.log(args["1"].tags["msg-id"])
                break;
            }
            case "CLEARCHAT": {
                console.log("Ban Perhamps?")
                console.log(args)
                break;
            }
            default : {
                console.log("No Clue, here's the args:")
                console.log(args)
                break;
            }
        }
        
        // const msg = document.createElement("div");
        // msg.className = "chat-message";
        // const colon = document.createElement("span");
        // colon.innerText = message;
        // msg.insertBefore(colon, null);
        // document.getElementById("chatbox").insertBefore(msg, null);
        // window.scrollBy({ 'top': window.innerHeight, 'behavior': 'instant' });
    });

    client.on('message', (channel, tags, message, self) => {
        client.log.info("Message");
        const config = state.config;
        const badgesData = state.badges;
        const twitchEmotes = state.emotes;
        const customEmotes = state.customEmotes;

        function getBadgesFor(name) {
            return (badgesData.find(setIdEquals(name)) ?? {"set_id": name, "versions": []});
        }

        const subscriberBadges = getBadgesFor("subscriber");
        const bitsBadges = getBadgesFor("bits");
        const emoteCache = twitchEmotes;

        const msg = document.createElement("div");
        if (!config.emoteOnly) {

            const badges = document.createElement("div");
            for (const badge in tags.badges) {
                let num = tags.badges[badge];

                var badgeNode = document.createElement("div");
                badgeNode.innerText = badge;
                switch (badge) {
                    case "subscriber": {
                        let imgData = subscriberBadges.versions.find((value, index, obj) => value.id == num);
                        
                        let imgSet = `${imgData.image_url_1x} 18w, ${imgData.image_url_2x} 36w, ${imgData.image_url_4x} 72w`;
                        const img = document.createElement("img");
                        img.src = imgData.image_url_4x;
                        img.srcset = imgSet;
                        badgeNode = img;
                        break;
                    }
                    case "bits": {
                        let imgData = bitsBadges.versions.find((value, index, obj) => value.id == num);
                        let imgSet = `${imgData.image_url_1x} 18w, ${imgData.image_url_2x} 36w, ${imgData.image_url_4x} 72w`;
                        const img = document.createElement("img");
                        img.src = badge.image_url_4x;
                        img.srcset = imgSet;
                        badgeNode = img;
                        break;
                    }
                    default:
                        let badgeData = badgesData.find(setIdEquals(badge));
                        if (badgeData) {
                            let imgData = badgeData.versions.find(IdEquals(num));
                            if (imgData) {
                                let imgURL = imgData.image_url_4x;
                                let imgSet = `${imgData.image_url_1x} 18w, ${imgData.image_url_2x} 36w, ${imgData.image_url_4x} 72w`;
                                const img = document.createElement("img");
                                img.src = imgURL;
                                img.srcset = imgSet;
                                badgeNode = img;
                            }
                        }
                        break;
                }
                badges.insertBefore(badgeNode, null);
            }

            // Chatter Name :)
            const name = document.createElement("span");
            name.innerText = tags['display-name'];
            name.style = `color:${tags.color}`;

            name.className = "name";
            badges.className = "badges";

            msg.insertBefore(name, null);
            msg.insertBefore(badges, name);
        }

        const text = document.createElement("span");
        text.className = "message"

        const colon = document.createElement("span");
        colon.innerText = config.emoteOnly ? "" : ":";
        text.insertBefore(colon, null);

        let lastElement = null;
        if (config.enableEmotes) {
            let emotesToReplace = [];
            if (tags.emotes && !!config.emotesTwitch) {
                for (const emote in tags.emotes) {
                    let locations = tags.emotes[emote];
                    let image = emoteCache.find(IdEquals(emote));
                    var imgURL = "";

                    let theme = "light";
                    // let imgScale = "3.0";
                    // imgURL = `https://static-cdn.jtvnw.net/emoticons/v2/${emote}/default/${theme}/${imgScale}`;

                    imgSet = [];
                    for (const scale of ["1", "2", "3"]) {
                        imgSet.push(`https://static-cdn.jtvnw.net/emoticons/v2/${emote}/default/${theme}/${scale}.0 ${scaleLookup[scale + "x"]}`);
                    }

                    locations.forEach((value, index, array) => {
                        let [start, end] = value.split("-")
                        emotesToReplace.push([parseInt(end), parseInt(start), imgSet]);
                    });
                }
            }

            if (config.customEmotes) {
                let replaceEmotesReverse = []
                for (const index in customEmotes) {
                    let emote = customEmotes[index];
                    let position = 0;
                    do {
                        position = message.indexOf(emote.name, position)

                        if (position != -1) {
                            let [start, end] = [position, position + emote.name.length];
                            replaceEmotesReverse.push([end, start, emote.imgSet]);
                            position = end;
                        }
                    } while (position != -1);
                    position = 0;
                }
                emotesToReplace = emotesToReplace.concat(replaceEmotesReverse.reverse());
            }

            for (const triple in emotesToReplace.sort((a, b) => { return b[0] - a[0] })) {
                let [end, start, imgSet] = emotesToReplace[triple];
                const endTxt = message.slice(end + 1);
                message = message.slice(0, start);

                if (!config.emoteOnly) {
                    if (endTxt) {
                        const txt = document.createElement("span");
                        txt.innerText = endTxt;
                        text.insertBefore(txt, lastElement);
                        lastElement = txt;
                    }
                };
                if (config.enableEmotes) {
                    const emote = document.createElement("img");
                    emote.srcset = imgSet;
                    text.insertBefore(emote, lastElement);
                    lastElement = emote;
                };

            }
        }
        if (!config.emoteOnly) {
            if (message) {
                const txt = document.createElement("div");
                txt.innerText = message;
                text.insertBefore(txt, lastElement);
            }
        }

        msg.className = "chat-message";
        msg.insertBefore(text, null);

        document.getElementById("chatbox").insertBefore(msg, null);
        window.scrollBy({ 'top': window.innerHeight, 'behavior': 'instant' });

    });
}