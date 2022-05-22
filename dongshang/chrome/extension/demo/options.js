
function GetAllTabs() {
  chrome.tabs.query(
    {currentWindow: true},
    function(tabs){
      console.log(tabs);
      let t = document.getElementById("all_tabs_textarea");
      t.value = JSON.stringify(tabs);
    }
  );
}

function GetAllFrames() {
  let tab_id =  document.getElementById("tab_id").value;
  chrome.webNavigation.getAllFrames(
        {"tabId":parseInt(tab_id)},
        function(details) {
          console.log(details);
          let t = document.getElementById("all_frames_textarea");
          t.value = JSON.stringify(details);
        }
  );
}

function ShowTab() {
  let info ="";
  info += location.href + "\n";
  info += document.title+ "\n";
  console.log(info);
}

function ExecuteScript() {
  chrome.tabs.query(
    {currentWindow: true},
    function(tabs){
      
      for(let i = 0; i < tabs.length; i++) {
        chrome.scripting.executeScript(
        {
          target: {tabId: tabs[i].id},
          func: ShowTab,
        },
        () => {  });
      
      }
    }
  );
}

// Add a button to the page for each supplied color
function constructOptions(buttonColors) {
  document.getElementById("all_tabs_button").onclick = GetAllTabs;
  document.getElementById("all_frames_button").onclick = GetAllFrames;
  document.getElementById("execute_script").onclick = ExecuteScript;
}

// Initialize the page by constructing the color options
constructOptions();