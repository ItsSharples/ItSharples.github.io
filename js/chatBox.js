// Assume the popup by the menubar
const isPopup = !window.menubar.visible;
// Does the window have a hash (The response metadata-ish bit)
if(window.location.hash)
{
    // Break the hash down into a dict (From https://stackoverflow.com/a/47628324)
    var params = {}
    window.location.hash.substring(1).split('&').map(hk => {
        let temp = hk.split('=');
        params[temp[0]] = temp[1]
    });

    if(isPopup) {
        localStorage.setItem("access_token", params.access_token);
        window.close();
    }
    chatVisuals(params);
}
else
{
    if(localStorage.getItem("access_token"))
    {
        configVisuals();
    }
    else
    {
        signInVisuals();
    }
    
}

function configVisuals() {
    document.getElementById("auth-content").innerHTML = "CONFIG MODE"
}

function chatVisuals(params) {

    console.log(params); //Here are the params to use

    document.getElementById("auth-content").innerHTML = "Chatbox mode" + params.access_token.toString()
}

document.getElementById("auth-button").onclick = () => {authenticate();}
function authenticate() {
    const url = "https://id.twitch.tv/oauth2/authorize";
    const client_id = "kf48fc5oafct9wqb1jf3lrsfurujq2";
    const redirect_uri = "https://itssharples.github.io/chatbox";
    const scope = "chat%3Aread";
    const state = "ThisIsSpooky";
    window.open(url + "?response_type=token" +
        "&client_id=" + client_id +
        "&redirect_uri=" + redirect_uri + 
        "&scope=" + scope +
        "&state=" + state,
        "Authenticate with Twitch",
        'scrollbars=no,resizable=no,status=no,location=no,toolbar=no,menubar=no'
    )
}