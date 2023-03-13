


const tableDOM = document.querySelector("table")
const exampleModeCheck = document.getElementById("example-mode");

let tableState = {}

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/random
function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max-min) + min);
  }
function getRandomAssignmentGrade() {
    const num = getRandomInt(20, 55)
    if(num > 50){return 0}
    return num * 2;
}
function getRandomName(){
    names = [
        "Cora Hurley",
        "Jerome Goodwin",
        "Alijah Roth",
        "Arturo House",
        "Cecilia Gomez",
        "Eleanor Berry",
        "Ashtyn Warren",
        "Josh Hicks",
        "Axel Orr",
        "Coby Arias",
        "Bronson Steele",
        "Nikhil Dalton",
        "Marlie Davila",
        "Darion Mullen",
        "Adriana Crosby",
        "Hassan Gaines",
        "Casey Williamson",
        "Javon Gonzales",
        "Keely Yates",
        "Jocelyn Pena",
        "Simeon Cunningham",
        "Ava Oneal",
        "Kenyon Deleon",
        "Libby Chambers",
        "Lila Howell",
        "Anaya Lewis",
        "Noelle Mason",
        "Mohammed Williamson",
        "Allie Walters",
        "Samuel Lambert",
        "Holly Nichols",
        "Brooklynn Ellison",
        "Jillian Shea",
        "Shirley Hanson",
        "Turner Knox",
        "Conor Pace",
        "George Snyder",
        "Clay Padilla",
        "Jordin Mooney",
        "Colten Hodges",
        "Kylan Reed",
        "Leland Yang",
        "Reece Harrington",
        "Reyna Morgan",
        "Ciara Kline",
        "Slade Brooks",
        "Blaine Beltran",
        "Peter Joyce",
        "Orlando Stevenson",
        "Hezekiah Merritt"
    ]
    let firstNames = []
    let lastNames = []
    for(name of names){
        splitNames = name.split(" ")
        firstNames.push(splitNames[0])
        lastNames.push(splitNames[1])
    }
    CharName = [firstNames[getRandomInt(0, firstNames.length)], lastNames[getRandomInt(0, firstNames.length)]].join(" ")
    return CharName;
}

const getRandomStudentID = () => getRandomInt(2100000, 2199999);

function resetTable(){
    console.log("Reset Table")
    tableState = {
        "Headers" : [
            "StudentName",
            "StudentID",
        ],
        "NumAssignments" : 5,
        "Contents" : []
    }

    for(newChar of [...Array(10)]){
        AssignmentGrades = [...Array(tableState.NumAssignments)].map((value, index, array)=>getRandomAssignmentGrade());
        tableState.Contents.push([getRandomName(), getRandomStudentID(), AssignmentGrades])
    }

    regenerateTableDOM()
}

function updateData() {
    console.log("Update Data")
}


function saveTable() {
    console.log("Save Table")
    localStorage.setItem("data", JSON.stringify(tableState));
}

function loadTable() {
    console.log("Load Table")
    tableState = JSON.parse(localStorage.getItem("data"));
    regenerateTableDOM()

}
function addRow(name, id) {
    console.log(`Add Row: ${name}`)
    if (!name || !id) {
        console.log("Invalid Inputs")
        return;
    }

    let AssignmentGrades = []
    if(exampleModeCheck.value == "on"){
        for(val of [...Array(tableState.NumAssignments)]){
            AssignmentGrades.push(getRandomAssignmentGrade())
        }
    }

    tableState.Contents.push([name, id, AssignmentGrades])
    regenerateTableDOM()

}
function addColumn(name = "", defaultValue = null) {
    if(name == ""){
        name = `Assignment ${tableState.NumAssignments + 1}`
    }
    console.log(`Add Column: ${name}`)
    tableState.NumAssignments += 1;

    if(exampleModeCheck.value == "on"){
        for(character of tableState.Contents){
            character[2].push(getRandomAssignmentGrade())
        }
    }

    regenerateTableDOM();
}

function clearTableDOM(){
    console.log("Clear Table DOM")
    while (tableDOM.firstChild) {
        // The list is LIVE so it will re-index each call
        tableDOM.removeChild(tableDOM.firstChild);
      }
}
// Doesn't work :angy:
function moveFocusEnd(cell) {
    console.log(cell)
    cell.focus()
    var tmp = cell.firstChild.textContent;
    cell.firstChild.textContent = '';
    cell.firstChild.textContent = tmp;
}
function onEditGrade(event){
    console.log("Edit Grade");
    var newVal = Number.parseInt(event.target.firstChild.data)
    if(isNaN(newVal)){
        event.preventDefault()
        console.log(`Invalid Input ${newVal}`)
        return
    }
    newVal = Math.max(newVal, 0);
    newVal = Math.min(newVal, 100);
    newVal = newVal == 0 ? "" : newVal;
    {
        // This messes up where the selection is, but meh
        event.target.firstChild.data = newVal

        const changedRow = event.target.parentElement.rowIndex - 1
        const changedCol = event.target.cellIndex - 2;
        tableState.Contents[changedRow][2][changedCol] = newVal;
        console.log(tableState);
        regenerateTableDOM([changedRow, changedCol])
    }
}

function getGradeText(percentGrade, type = "percent"){
    const mapTo = {
            60 : ["F", "0.0"],
            62 : ["D-", "0.7"],
            66 : ["D", "1.0"],
            69 : ["D+", "1.3"],
            72 : ["C-", "1.7"],
            66 : ["C", "2.0"],
            69 : ["C+", "2.3"],
            82 : ["B-", "2.7"],
            86 : ["B", "3.0"],
            89 : ["B+", "3.3"],
            92 : ["A-", "3.7"],
            100 : ["A", "4.0"]
    }
    selectedGrade = []
    for(key of Object.keys(mapTo)){
        if (Number.parseInt(key) > percentGrade)
        {
            selectedGrade = mapTo[key]
            break;
        }
    }
    switch (type){
        case "percent":
            return percentGrade;
        case "letter":
            return selectedGrade[0];
        case "number":
            return selectedGrade[1];
    } 
    
}
function getGrade(values, type = "percent"){
    var percent = values.reduce((acc, curr) => acc + curr, 0) / tableState.NumAssignments;
    percent = Math.round(percent);


    return {"Pass" : percent > 60, "Text": getGradeText(percent, type)};

}

function makeButton(name, id = "", clickEvent = null){
    const newButton = document.createElement("button");
    newButton.id = id;
    newButton.textContent = name

    newButton.addEventListener("click", clickEvent)
    return newButton
}

// https://developer.mozilla.org/en-US/docs/Web/API/Document_Object_Model/Traversing_an_HTML_table_with_JavaScript_and_DOM_Interfaces
// https://javascript.plainenglish.io/how-to-listen-for-changes-to-html-elements-with-the-contenteditable-attribute-with-javascript-b715eeb4431e


/* https://www.matuzo.at/blog/highlighting-columns/ */
/* https://stackoverflow.com/a/42658933 */
function regenerateTableDOM(focusOn = []){
    console.log("Regenerate Table DOM")
    const tblBody = document.createElement("tbody");
    const tblHead = document.createElement("thead");
    const colGroup = document.createElement("colgroup");


    const gradeSelect = document.createElement("select");

    let shouldFocus = focusOn != [];
    let focusCell = null;

    // generate the headers
    const headerRow = document.createElement("tr");
    for (const rowData of tableState.Headers) {
        const cell = document.createElement("th");
        const cellText = document.createTextNode(rowData);
        cell.appendChild(cellText);
        headerRow.appendChild(cell);

        const col = document.createElement("col");
        col.classList.add(`${rowData}-col`);
        colGroup.appendChild(col);
    }
    // Add the Assignment Headers
    Array(tableState.NumAssignments).fill().map((_, assignmentNum) => {
        const cell = document.createElement("th");
        const cellText = document.createTextNode(`Assignment ${assignmentNum + 1}`);
        cell.appendChild(cellText);
        headerRow.appendChild(cell);

        const col = document.createElement("col");
        col.classList.add("assignment-col");
        colGroup.appendChild(col);
    }) 

    // The Final Grade Column is going to be funky, so just add it specifically
    {
        const gradeCell = document.createElement("th");
        const cellText = document.createTextNode("Final Grade");

        const gradeLabel = document.createElement("label");
        //gradeSelect = document.createElement("select");
        
        const gradePercent = document.createElement("option");
        gradePercent.value = "percent";
        gradePercent.textContent = "Average [%]";
        const gradeLetter = document.createElement("option");
        gradeLetter.value = "letter";
        gradeLetter.textContent = "Average [Letter]";
        const gradeNumber = document.createElement("option");
        gradeNumber.value = "number";
        gradeNumber.textContent = "Average [4.0]";

        gradePercent.setAttribute("selected", true);

        gradeSelect.classList.add("grade-selector")
        gradeSelect.add(gradePercent);
        gradeSelect.add(gradeLetter);
        gradeSelect.add(gradeNumber);

        //gradeCell.appendChild(cellText);
        gradeCell.appendChild(gradeSelect);
        headerRow.appendChild(gradeCell);

        const col = document.createElement("col");
        col.classList.add("final-grade-col");
        colGroup.appendChild(col);
    }

    // Add the headers to the body
    tblHead.appendChild(headerRow);
    /// Now we add the Contents to the Table
    for (const rowData of tableState.Contents) {
        const row = document.createElement("tr");
        const focusOnRow = shouldFocus & (focusOn[0] == tblBody.childElementCount);
        
        let gradeValues = []
        for (const columnData of rowData) {
            
            // If it's a list, assume it be the assignment values
            if(Object.prototype.toString.call(columnData) == "[object Array]"){
                gradeValues = columnData
                Array(tableState.NumAssignments).fill().map((_, assignmentNum) => {
                    const cell = document.createElement("td");
                    const cellText = document.createTextNode("");

                    const scoreGot = gradeValues[assignmentNum];
                    cellText.textContent = !!scoreGot ? scoreGot : "";
                    cell.classList.add("grade")

                    cell.appendChild(cellText);
                    row.appendChild(cell);

                    // https://stackoverflow.com/a/20906991
                    // Add Content Editable Attribute
                    cell.contentEditable = "true";
                    cell.inputMode = "numeric"

                    cell.addEventListener("input", onEditGrade);

                    const focusOnCell = focusOnRow & (focusOn[1] == assignmentNum)
                    if (focusOnCell){focusCell = cell; shouldFocus = false;}
                }) 
            }
            else {
                const cell = document.createElement("td");
                const cellText = document.createTextNode("");
                cellText.textContent = columnData;

                cell.appendChild(cellText);
                row.appendChild(cell);

                if(columnData == rowData[0]){
                    cell.addEventListener("click", (event) => {
                        const didntContain = !event.target.parentElement.classList.contains("highlight-click-row");
                        for(element of document.getElementsByClassName("highlight-click-row")){
                            element.classList.remove("highlight-click-row");
                        }

                        if(didntContain){
                            event.target.parentElement.classList.add("highlight-click-row")
                        }
                    })
                }
            }
        }

        // Work out the final Grade
        const gradeCell = document.createElement("td");
        
        const cellText = document.createTextNode("2");

        function updateGradeText(){
            const grade = getGrade(gradeValues, gradeSelect.value);
            cellText.textContent = grade.Text
            if(grade.Pass){gradeCell.classList.remove("fail-grade");}
            else{gradeCell.classList.add("fail-grade");}
            
        }

        updateGradeText()
        gradeSelect.addEventListener("change", (event) => {
            updateGradeText()
        });

        gradeCell.appendChild(cellText);
        gradeCell.classList.add("grade")
        row.appendChild(gradeCell);

        // add the row to the end of the table body
        tblBody.appendChild(row);
    }

    {// Add footer to add the add buttons
        const footer = document.createElement("tfoot");
        const dialog = document.createElement("dialog");
        { // Create Student Dialog
            const title = document.createElement("p");
            const form = document.createElement("form");
            form.method = "dialog";

            const studentNameLabel = document.createElement("label");
            studentNameLabel.htmlFor = "studentName"
            studentNameLabel.textContent = "Student Name";
            const studentIDLabel = document.createElement("label");
            studentIDLabel.htmlFor = "studentID"
            studentIDLabel.textContent = "Student ID";
            const studentName = document.createElement("input");
            studentName.name = "studentName"
            const studentID = document.createElement("input");
            studentID.name = "studentID"
            const submit = document.createElement("button");
            submit.textContent = "OK";
            submit.type = "submit"

            form.title = "New Student"
            form.appendChild(studentNameLabel);
            form.appendChild(studentName);
            form.appendChild(studentIDLabel);
            form.appendChild(studentID);
            form.appendChild(submit);
            form.submit = submit;
            form.onsubmit = (submit) => {
                console.log(submit);
                let name = "";
                let id = "";
                for(child of submit.target){
                    if (child.name == "studentID") {id = child.value}
                    if (child.name == "studentName") {name = child.value}
                }
                addRow(name, id)
                dialog.close();
                
            }

            dialog.appendChild(title);
            dialog.appendChild(form);
        }

        const studentCell = document.createElement("td");
        const addStudentButton = makeButton("Add Student", "new-student-btn", (event) => {
            if(exampleModeCheck.value == "on"){
                addRow(getRandomName(), getRandomStudentID())
            }
            else{
                dialog.show()
            }
        });
        addStudentButton.appendChild(dialog);
        studentCell.appendChild(addStudentButton);
        footer.appendChild(studentCell);

        const assignmentCell = document.createElement("td");
        const addAssignmentButton = makeButton("Add Assignment", "new-assignment-btn", (event)=>{addColumn();})
        assignmentCell.appendChild(addAssignmentButton);
        footer.appendChild(assignmentCell);

        const saveCell = document.createElement("td");
        const addSaveButton = makeButton("Save Table", "save-table-btn", (event)=>{saveTable();})
        saveCell.appendChild(addSaveButton);
        footer.appendChild(saveCell);

        const loadCell = document.createElement("td");
        const addLoadButton = makeButton("Load Table", "load-table-btn", (event)=>{loadTable();})
        loadCell.appendChild(addLoadButton);
        footer.appendChild(loadCell);

        const resetCell = document.createElement("td");
        const addResetButton = makeButton("Reset Table", "reset-table-btn", (event)=>{resetTable();})
        resetCell.appendChild(addResetButton);
        footer.appendChild(resetCell);

        tblBody.appendChild(footer);
    }


    // Actually write this to the DOM
    clearTableDOM();
    tableDOM.appendChild(colGroup);
    tableDOM.appendChild(tblHead);
    tableDOM.appendChild(tblBody);
    if(!!focusCell){
        
        moveFocusEnd(focusCell);
    }

    // Add Hover Effect 
    {
        const cols = tableDOM.getElementsByTagName('col');
        const tds = document.getElementsByTagName('td');
        const ths = document.getElementsByTagName('th');
        const columnEnter = (i) => {if(!!cols[i]){cols[i].classList.add('highlight-hover');}}
        const columnLeave = (i) => {if(!!cols[i]){cols[i].classList.remove('highlight-hover');}}

        for (const cell of [...tds].concat([...ths])) {
            const index = cell.cellIndex;
            cell.addEventListener('mouseenter', columnEnter.bind(this, index));
            cell.addEventListener('mouseleave', columnLeave.bind(this, index));
        }
        for (const cell of [...ths]) {
            const index = cell.cellIndex;
            if(!!cols[index]){
            if(cols[index].classList.contains("assignment-col")) {
            cell.addEventListener('click', (event) => {
                
                    const col = cols[index];
                    const didntContain = !col.classList.contains("highlight-click-col");
                    for(element of document.getElementsByClassName("highlight-click-col")){
                        element.classList.remove("highlight-click-col");
                    }

                    if(didntContain){
                        col.classList.add("highlight-click-col")
                    }
                }
            );
        }
    }
    }
}
}



function start() {
    console.log("Starting")

    // Check if there's any local storage
    if(!!localStorage.getItem("data")){
        loadTable();
    }
    else{
        resetTable();
        saveTable();
    }

    // const observer = new MutationObserver((mutationRecords) => {
        
    //     const target = mutationRecords[0].target;
    //     console.log(target.data)
    //     //target.data = Number.parseInt(target.data);
        
    //     updateData()
    // })
    // console.log(tableDOM)
    // observer.observe(tableDOM, {
    // characterData: true,
    // subtree: true,
    // })

    regenerateTableDOM()
}
resetTable()
start()