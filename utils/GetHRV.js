const fs = require("fs");
const { promisify } = require("util");
const writeFile = promisify(fs.writeFile);
const mkdir = promisify(fs.mkdir);
const rmdir = promisify(fs.rmdir);
const exec = promisify(require('child_process').exec);

// implements the interface with the get_hrv cli 
// input is beat to beat data, output is obj with computed values (e.g. SDNN, rMSSD, HFPWR, etc.)
// supports concurrency by running in different working directories
async function get_hrv(hrvInput, date) {
    return new Promise(async (resolve, reject) => {
        let nnArray = hrvInput.map((entry, index) => {
            if (index < 1) { entry.nn = entry.timeSinceSeriesStart * 1000 }
            else { entry.nn = (entry.timeSinceSeriesStart - hrvInput[index - 1].timeSinceSeriesStart) * 1000 }
            return entry
        })

        //remove the values preceded by gaps and discard data other than NN
        let nnArrayFiltered = nnArray.filter((entry, index) => !entry.precededByGap && index > 0).map(entry => entry.nn)
        //console.log(nnArrayFiltered)

        //create a string for writing to file
        let nnString = nnArrayFiltered.reduce((string, time) => string + time + '\n', '')

        //write the file for purposes of running the get_hrv CLI 
        let timestamp = new Date().getTime()
        let rand = Math.floor(Math.random() * 100)
        let fileName = `NN_${timestamp}.rr`
        let path = `./${timestamp}_${rand}`

        let hrvObject = {}; //object to return

        try {
            await mkdir(path, { recursive: true }) //make a unique dir for executing get_hrv
            await writeFile(`${path}/${fileName}`, nnString, 'utf8') //put NN file in dir
            console.log(`The NN file ${fileName} has been saved!`);
            //execute the get_hrv CLI once the file has been written, change working dir
            const { stdout, stderr } = await exec(`get_hrv -M -m -R ${fileName}`, { cwd: path })
            console.log(stdout);
            
            //parse stdout
            stdout.split('\n').map((line) => line.split('=')) //"SDNN     = 0.583836" => ["SDNN     ", " 0.583836"]
                .forEach(lineArr => {
                    if (lineArr.length === 2) {
                        hrvObject[lineArr[0].trim().replace(' ', '').replace('/', 'to')] = lineArr[1].trim() //["SDNN     ", " 0.583836"] => {"SDNN": "0.583836"}
                    }
                });
            if (Object.keys(hrvObject).length > 1) {
                hrvObject.createdAt = date; //set the date passed in
                console.log(hrvObject);
            }
            else {
                console.log("error", "hrvobject is empty")
                hrvObject = null
            }
        }
        catch (err) {
            reject(console.log(err))
        }

        //cleanup
        try {
            await rmdir(path, { recursive: true }) //cleanup
        }
        catch (err) {
            console.log(err)
        }
        resolve(hrvObject)
    })
}

module.exports = get_hrv