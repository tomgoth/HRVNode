// const hrvSeries = [{timeSinceSeriesStart: 1.625, precededByGap: false}, ---- sample data ----
// {timeSinceSeriesStart: 26.67578125, precededByGap: true},
// {timeSinceSeriesStart: 27.6640625, precededByGap: false},
// {timeSinceSeriesStart: 28.73046875, precededByGap: false},
// {timeSinceSeriesStart: 29.7578125, precededByGap: false},
// {timeSinceSeriesStart: 30.7421875, precededByGap: false},
// {timeSinceSeriesStart: 31.6875, precededByGap: false},
// {timeSinceSeriesStart: 32.60546875, precededByGap: false},
// {timeSinceSeriesStart: 33.5625, precededByGap: false},
// {timeSinceSeriesStart: 34.5546875, precededByGap: false},
// {timeSinceSeriesStart: 35.6015625, precededByGap: false},
// {timeSinceSeriesStart: 36.6484375, precededByGap: false},
// {timeSinceSeriesStart: 37.69921875, precededByGap: false},
// {timeSinceSeriesStart: 38.72265625, precededByGap: false},
// {timeSinceSeriesStart: 39.70703125, precededByGap: false},
// {timeSinceSeriesStart: 40.6953125, precededByGap: false},
// {timeSinceSeriesStart: 41.74609375, precededByGap: false},
// {timeSinceSeriesStart: 42.74609375, precededByGap: false},
// {timeSinceSeriesStart: 43.6875, precededByGap: false},
// {timeSinceSeriesStart: 44.62109375, precededByGap: false},
// {timeSinceSeriesStart: 45.609375, precededByGap: false},
// {timeSinceSeriesStart: 46.5546875, precededByGap: false},
// {timeSinceSeriesStart: 47.484375, precededByGap: false},
// {timeSinceSeriesStart: 48.53125, precededByGap: false},
// {timeSinceSeriesStart: 57.66796875, precededByGap: true},
// {timeSinceSeriesStart: 58.6875, precededByGap: false},
// {timeSinceSeriesStart: 59.7734375, precededByGap: false},
// {timeSinceSeriesStart: 60.8671875, precededByGap: false},
// {timeSinceSeriesStart: 61.96484375, precededByGap: false},
// {timeSinceSeriesStart: 63.02734375, precededByGap: false},
// {timeSinceSeriesStart: 64.046875, precededByGap: false}]

const express = require('express');
const fs = require('fs');
const app = express();
const exec = require('child_process').exec;
const mongo = require('mongoose')
const dotenv = require('dotenv')
const HRVReading = require('./schemas/HRVReading')
const asyncHandler = require('./middleware/async')
const cors = require('cors')
const https = require('https');

dotenv.config({path: './config.env'})

mongo.connect(process.env.MONGO_URI, {
    useNewUrlParser: true,
    useCreateIndex: true,
    useFindAndModify: false,
    useUnifiedTopology: true
})

app.listen(3001, () => {
 console.log("Server running on port 3001");
});

app.use(cors())

app.use(express.json());

app.post("/gethrv", (req, res, next) => {
    //console.log(req.body)
    let hrvInput = req.body.beatToBeat;
    
    //calculate NN data
    let nnArray = hrvInput.map((entry, index) => {
        if (index < 1)  { entry.nn = entry.timeSinceSeriesStart*1000 }
        else { entry.nn = (entry.timeSinceSeriesStart - hrvInput[index - 1].timeSinceSeriesStart)*1000 }
        return entry
    }) 
    
    //remove the values preceded by gaps and discard data other than NN
    let nnArrayFiltered = nnArray.filter((entry, index) => !entry.precededByGap && index > 0).map(entry => entry.nn)
    console.log(nnArrayFiltered)

    //create a string for writing to file
    let nnString = nnArrayFiltered.reduce((string, time) => string + time + '\n' ,'')

    //write the file for purposes of running the get_hrv CLI
    fs.writeFile('NN.rr', nnString, 'utf8', (err) => {
        if (err) throw err;
        console.log('The NN file has been saved!');
        //execute the get_hrv CLI once the file has been written
        exec('get_hrv -M -m -R NN.rr', (error, stdout, stderr) => { //get_hrv must be installed on the machine
            if (error) throw error;
            console.log(stdout);
            let hrvObject = {}; //object to return
            //parse stdout
            stdout.split('\n').map((line) => line.split('=')) //"SDNN     = 0.583836" => ["SDNN     ", " 0.583836"]
                .forEach(lineArr => {
                    if (lineArr.length === 2) {  
                        hrvObject[lineArr[0].trim().replace(' ','').replace('/','to')] = lineArr[1].trim() //["SDNN     ", " 0.583836"] => {"SDNN": "0.583836"}
                    }
            });
            hrvObject.createdAt = req.body.date;
            console.log(hrvObject);
            res.json(hrvObject);
            HRVReading.create(hrvObject)
        });
         
    });
    
});

app.get("/recent", asyncHandler(async (req, res, next) => {
    //get most recent readings, parameterize how many to get

    const readings = await HRVReading.find({}).sort({createdAt: 1})

    res.status(200).json({
        success: true,
        count: readings.length,
        data: readings
    });
    
}));