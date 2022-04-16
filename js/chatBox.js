
function reset() {
    document.getElementById("config-body").hidden = true;
    document.getElementById("auth-body").hidden = true;
}
function refresh() {
    // Assume the popup by the menubar
    const isPopup = (window.opener && window.opener !== window) || !window.menubar.visible;
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

    if(isPopup) {
        window.close();
    }
}

function chatVisuals(params) {

    console.log(params); //Here are the params to use

    document.getElementById("auth-content").innerHTML = "Chatbox Mode";
}


function configVisuals() {
    document.getElementById("config-body").hidden = false;

    document.getElementById("auth-content").innerHTML = "Access Token: " + localStorage.getItem("access_token").toString();
}


function signInVisuals() {
    document.getElementById("auth-body").hidden = false;

    document.getElementById("auth-button").onclick = () => {authenticate();}
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

const b64toBlob = (b64Data, contentType='', sliceSize=512) => {
    const byteCharacters = atob(b64Data);
    const byteArrays = [];
  
    for (let offset = 0; offset < byteCharacters.length; offset += sliceSize) {
      const slice = byteCharacters.slice(offset, offset + sliceSize);
  
      const byteNumbers = new Array(slice.length);
      for (let i = 0; i < slice.length; i++) {
        byteNumbers[i] = slice.charCodeAt(i);
      }
  
      const byteArray = new Uint8Array(byteNumbers);
      byteArrays.push(byteArray);
    }
  
    const blob = new Blob(byteArrays, {type: contentType});
    return blob;
  }

function getFormJSON(form) {
    const data = new FormData(form);
    return Array.from(data.keys()).reduce((result, key) => {
        if (result[key]) {
        result[key] = data.getAll(key)
        return result
        }
        result[key] = data.get(key);
        return result;
    }, {});
};
function JSONtoBase64(json) {
    return btoa(JSON.stringify(json));
}
function Base64ToJSON(b64Data) {
    return JSON.parse(atob(b64Data));
}
async function updateDemotext()  {
    document.getElementById("demo").style.setProperty("--demo-family", document.getElementById("fontFamily").value);
    document.getElementById("demo").style.setProperty("--demo-size", document.getElementById("fontSize").value + "px");
    
    document.getElementById("preview").style.setProperty("--font-family", document.getElementById("fontFamily").value);
    document.getElementById("preview").style.setProperty("--font-size", document.getElementById("fontSize").value + "px");
    
    // const objson = getFormJSON(document.getElementById("config-form"));
    // const b64 = JSONtoBase64(objson);
    // const json = Base64ToJSON(b64);
    // console.log(objson);
    // console.log(b64);
    // console.log(json);
}

function HexEncode(str){
    var hex, i;

    var result = "";
    for (i=0; i<str.length; i++) {
        hex = str.charCodeAt(i).toString(36);
        result += ("000"+hex).slice(-4);
    }

    return result
}

function HexDecode(str){
    var j;
    var hexes = str.match(/.{1,4}/g) || [];
    var back = "";
    for(j = 0; j<hexes.length; j++) {
        back += String.fromCharCode(parseInt(hexes[j], 36));
    }

    return back;
}

document.getElementById("fontFamily").addEventListener('input', ()=>{updateDemotext();});
document.getElementById("fontSize").addEventListener('input', ()=>{updateDemotext();});
window.addEventListener('storage', () => {reset();refresh();});
refresh();