
function reset() {
    document.getElementById("config-body").hidden = true;
    document.getElementById("auth-body").hidden = true;
}
function refresh() {
    // Assume the popup by the menubar
    const isPopup = (window.opener && window.opener !== window) || !window.menubar.visible;
    // Does the window have a hash (The response metadata-ish bit)
    if (window.location.hash) {
        // Break the hash down into a dict (From https://stackoverflow.com/a/47628324)
        var params = {}
        window.location.hash.substring(1).split('&').map(hk => {
            let temp = hk.split('=');
            params[temp[0]] = temp[1]
        });

        if (isPopup) {
            localStorage.setItem("access_token", params.access_token);
        }
    }
    else {
        // if (localStorage.getItem("access_token")) {
        //     configVisuals();
        // }
        // else {
        //     signInVisuals();
        // }
    }

    if (isPopup) {
        window.close();
    }
}

function chatVisuals(params) {

    console.log(params); //Here are the params to use

    document.getElementById("auth-content").innerHTML = "Chatbox Mode";
}


function updateConfig(config) {
    document.getElementById("auth-content").innerHTML = "Access Token: " + localStorage.getItem("access_token").toString();

    // let config = {};
    // config.accessToken = localStorage.access_token;
    // config.channelName = "sharples";

    // config.enableEmotes = true;
    // config.emotesTwitch = true;
    // config.emotesBTTV = true;
    // config.emotesFFZ = false;
    // config.emotes7TV = false;
    // config.emoteOnly = false;

    // config.fontFamily = "system-ui";

    // config.fontSize = 15;
    // config.badgeSize = 15;
    // config.emoteSize = 15;

    config = config;
    sessionStorage.config = ConfigToBase64(config);

    document.getElementById("chatbox-url").value = `${window.location.origin}/chatbox/chatbox.html#${ConfigToBase64(config)}`;
    document.getElementById("chatbox-url").style = "font-size: 15px; width: 100%";
}

function signInVisuals() {
    document.getElementById("auth-body").hidden = false;

    document.getElementById("auth-button").onclick = () => { authenticate(); }
}


function authenticate() {
    let url = "https://id.twitch.tv/oauth2/authorize";
    let client_id = "kf48fc5oafct9wqb1jf3lrsfurujq2";
    let redirect_uri = "https://itssharples.github.io/chatbox";
    let scope = "chat%3Aread";
    let state = Math.random().toPrecision(21).toString(36);
    window.open(url +
        "?response_type=token" +
        "&client_id=" + client_id +
        "&redirect_uri=" + redirect_uri +
        "&scope=" + scope +
        "&state=" + state,
        "Authenticate with Twitch",
        'scrollbars=no,resizable=no,status=no,location=no,toolbar=no,menubar=no,width=400,height=530'
    );
}

function JSONtoBase64(json) {
    return btoa(JSON.stringify(json));
}
function Base64ToJSON(b64Data) {
    return JSON.parse(atob(b64Data));
}

function ConfigToBase64(config) {
    output = {}
    output.token = config.accessToken;
    output.channel = config.channelName;

    output.emotes = config.enableEmotes;
    output.enabled = {
        'only' : config.emoteOnly,
        'twitch' : config.emotesTwitch,
        'bttv' : config.emotesBTTV,
        'ffz' : config.emotesFFZ,
        '7TV' : config.emotes7TV
    }
    output.font = {
        'family' : config.fontFamily,
        'font' : config.fontSize,
        'badge' : config.badgeSize,
        'emote' : config.emoteSize
    }
    return JSONtoBase64(output);
}
function Base64ToConfig(b64) {
    input = Base64ToJSON(b64);
    config = {};
    config.accessToken = input.token;
    config.channelName = input.channel;

    config.enableEmotes = input.emotes;
    config.emoteOnly = input.enabled.only;
    config.emotesTwitch = input.enabled.twitch;
    config.emotesBTTV = input.enabled.bttv;
    config.emotesFFZ = input.enabled.ffz;
    config.emotes7TV = input.enabled.stv;

    config.fontFamily = input.font.family; 
    config.fontSize = input.font.font; 
    config.badgeSize = input.font.badge; 
    config.emoteSize = input.font.emote; 

    return config;
}

function UpdateHTMLForm(config) {
    //eyJ0b2tlbiI6IjJ2dm10b2E1eDlqbmZkN3hhMmI1dmszZGdvcnRtYSIsImNoYW5uZWwiOiJzaGFycGxlcyIsImVtb3RlcyI6Im9uIiwiZW5hYmxlZCI6eyJvbmx5Ijp0cnVlLCJ0d2l0Y2giOiJvbiIsImJ0dHYiOiJvbiJ9LCJmb250Ijp7ImZhbWlseSI6InN5c3RlbS11aSIsImZvbnQiOiI1MCJ9fQ
    for(item in config)
    {
        let element = document.getElementsByName(item)[0];
        if(element) {
            element.value = config[item];
            if(element.type == "checkbox")
            {
                element.checked = !!config[item];
            }
        }

    }
}



async function updateDemotext() {
    console.log("Update");
    const fontSize = document.getElementById("fontSize").value;
    document.getElementById("demo").style.setProperty("--demo-family", document.getElementById("fontFamily").value);
    document.getElementById("demo").style.setProperty("--demo-size", fontSize + "px");

    document.getElementById("chatbox").style.setProperty("--font-family", document.getElementById("fontFamily").value);

    document.getElementById("chatbox").style.setProperty("--font-height", fontSize + "px");
    document.getElementById("chatbox").style.setProperty("--font-height-half", (fontSize / 2) + "px");
}
const changeVisibility = (element, value) => element.style.visibility = value ? 'visible' : 'hidden';
const changeDisplay = (element, value) => element.style.display = value ? 'initial' : 'none';
const setValue = (element, value) => element.value = value;
const addEventListenerAndFire = (element, type, func) => { element.addEventListener(type, func); element.dispatchEvent(new Event(type)); }



var config = {}
let load = false;

if(sessionStorage.getItem("config"))
{
    config = Base64ToConfig(sessionStorage.config);
    UpdateHTMLForm(config);
}

if (true) {
    // addEventListenerAndFire(document.getElementById("fontFamily"), 'input', () => { updateDemotext(); });
    // addEventListenerAndFire(document.getElementById("fontSize"), 'input', () => { updateDemotext(); });

    addEventListenerAndFire(document.getElementById("customSizes"), 'change', (event) => {
        document.getElementsByName("customSize").forEach((element) => {
            changeDisplay(element, event.currentTarget.checked);
            let inputNodes = element.getElementsByTagName("input");
            for (var i = 0; i < inputNodes.length; ++i) {
                var child = inputNodes[i];
                setValue(child, document.getElementById("fontSize").value);
            }
        })
    })

    addEventListenerAndFire(document.getElementById("enableEmotes"), 'change', (event) => {
        changeDisplay(document.getElementById("emoteList"), event.currentTarget.checked);
        changeDisplay(document.getElementById("emoteOnlyInput"), event.currentTarget.checked);
    })
    let form = document.getElementById("config-form");
    addEventListenerAndFire(form, 'change', (event) => {
        updateDemotext()
        submitForm(event);
    })

}

addEventListenerAndFire(window, 'storage', () => refresh())
refresh();


function submitForm(event) {
    event.preventDefault();

    const data = new FormData(document.getElementById("config-form"));
    const config = Object.fromEntries(data.entries());
    config.accessToken = localStorage.access_token;

    updateConfig(config)
}