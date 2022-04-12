
if(window.location.hash)
{
    runChat(window.location.hash.substring(1));
}
else
{
    runConfig();
}

function runConfig() {
    document.getElementById("subbody").innerHTML = "CONFIG MODE"
}

function runChat(hash) {
    var params = {}
    hash.split('&').map(hk => {
        let temp = hk.split('=');
        params[temp[0]] = temp[1]
    });
    console.log(params); //Here are the params to use

    document.getElementById("subbody").innerHTML = "Chatbot mode" + params.access_token.toString()
}

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
        "&state=" + state
    )
}