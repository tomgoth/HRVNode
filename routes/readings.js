const express = require('express')
const HRVReading = require('../schemas/HRVReading')
const RHRReading = require('../schemas/RHRReading')
const asyncHandler = require('../middleware/async')
const get_hrv = require('../utils/GetHRV')
const moment = require('moment')
const app = express.Router()
const auth = require('../middleware/auth');
const { mean, std, ln } = require('../utils/math')


app.post("/hrv", auth, asyncHandler(async (req, res, next) => {
    //compute hrv stats e.g. SDNN, rMSSD, HFPWR, etc. from beat to beat data
    const hrvObj = await get_hrv(req.body.beatToBeat, req.body.date)
    // return the obj even if it doesn't get stored (e.g. duplicate)
    if (hrvObj) res.status(201).json(hrvObj)
    // store the computed obj in our readings database
    if (hrvObj) await HRVReading.create({ ...hrvObj, user: req.user.id })

}))

app.get("/hrv/:page/:size", auth, asyncHandler(async (req, res, next) => {
    //get most recent readings, parameterize how many to get
    //TODO: smoothing, pagination

    const skips = parseInt(req.params.size) * (parseInt(req.params.page) - 1)
    const limit = parseInt(req.params.size)

    //compare to recent 6 weeks?
    let startDate = moment().subtract(6, 'week').toDate();

    let readings = await HRVReading.find({ user: req.user.id }).sort({ createdAt: -1 }).skip(skips).limit(limit)
    readings = await Promise.all(readings.map(reading => percentileHrvCalc(req, startDate, reading._doc)))
        
    res.status(200).json({
        success: true,
        count: readings.length,
        data: readings
    });


}));

app.get("/hrv/mostrecent", auth, asyncHandler(async (req, res, next) => {

    let mostRecentHRV = await HRVReading.findOne({ user: req.user.id }).sort({ createdAt: -1 })
    res.status(200).json(mostRecentHRV)

}))

app.post("/rhr", auth, asyncHandler(async (req, res, next) => {
    const saved = await RHRReading.create({
        user: req.user.id,
        restingHeartRate: req.body.heartrate,
        createdAt: req.body.timestamp
    })
    res.status(201).json(saved)
}))

app.get("/rhr", auth, asyncHandler(async (req, res, next) => {
    //get most recent readings, parameterize how many to get
    //TODO: smoothing, pagination
    let startDate = moment().subtract(6, 'week').toDate();


    const readings = await RHRReading.find({ user: req.user.id, createdAt: { $gte: startDate } }).sort({ createdAt: -1 })

    res.status(200).json({
        success: true,
        count: readings.length,
        data: readings
    });

}));

app.get("/rhr/mostrecent", auth, asyncHandler(async (req, res, next) => {

    let mostRecentRHR = await RHRReading.findOne({ user: req.user.id }).sort({ createdAt: -1 })
    res.status(200).json(mostRecentRHR)

}));

//Smallest worthwhile change 
app.get("/swc", asyncHandler(async (req, res, next) => {
    
    let baselineStart = moment().subtract(6, 'week').toDate();
    let weekStart = moment().subtract(1, 'week').toDate();

    const baselineReadings = await HRVReading.find({ user: '5eb3395194499571cefaeeb5', createdAt: { $gte: baselineStart } }).sort({ createdAt: -1 })
    const weekReadings = await HRVReading.find({ user: '5eb3395194499571cefaeeb5', createdAt: { $gte: weekStart } }).sort({ createdAt: -1 })
    console.log('number readings in baseline', baselineReadings.length)
    
    //rMSSD
    const blRMSSDMean = mean(baselineReadings.map(reading => { return ln(parseFloat(reading.rMSSD))} ))
    const blRMSSDStd = std(baselineReadings.map(reading => ln(parseFloat(reading.rMSSD))))
    const weekRMSSDMean = mean(weekReadings.map(reading => { return ln(parseFloat(reading.rMSSD))} ))

    //HFPWR


    // const isWithinSWC = (baselineMean - baselineStd * .5) <= weekMean 
    
    res.status(200).json({isWithinSWC})

}));

app.get("/readiness/:fromNowInt/:fromNowUnit", auth, asyncHandler(async (req, res, next) => {
    let fromNow = moment().subtract(req.params.fromNowInt, req.params.fromNowUnit).toDate()

    //compare to recent 6 weeks?
    let startDate = moment().subtract(6, 'week').toDate();

    let currentRHR
    const RHRsfromNow = await RHRReading.find({ user: req.user.id, createdAt: { $gte: fromNow } }).sort({ createdAt: 1 })
    if (RHRsfromNow[0]) {
        currentRHR = RHRsfromNow.reduce((totalObj, rhr) => {
            return {
                createdAt: rhr.createdAt,
                id: rhr.id,
                restingHeartRate: totalObj.restingHeartRate + rhr.restingHeartRate
            }
        })
        currentRHR = {
            createdAt: currentRHR.createdAt,
            id: currentRHR.id,
            restingHeartRate: currentRHR.restingHeartRate / RHRsfromNow.length
        }
    }
    else {
        currentRHR = await RHRReading.findOne({ user: req.user.id }).sort({ createdAt: -1 })
    }


    let currentHRV
    const HRVsfromNow = await HRVReading.find({ user: req.user.id, createdAt: { $gte: fromNow } }).sort({ createdAt: 1 })
    if (HRVsfromNow[0]) {
        currentHRV = HRVsfromNow.reduce((totalObj, hrv) => {
            return {
                createdAt: hrv.createdAt,
                id: hrv.id,
                SDNN: totalObj.SDNN + hrv.SDNN,
                rMSSD: totalObj.rMSSD + hrv.rMSSD,
                HFPWR: totalObj.HFPWR + hrv.HFPWR
            }
        })
        currentHRV = {
            createdAt: currentHRV.createdAt,
            id: currentHRV.id,
            SDNN: currentHRV.SDNN / HRVsfromNow.length,
            rMSSD: currentHRV.rMSSD / HRVsfromNow.length,
            HFPWR: currentHRV.HFPWR / HRVsfromNow.length
        }
    }
    else {
        currentHRV = await HRVReading.findOne({ user: req.user.id }).sort({ createdAt: -1 })
    }

    const [{ sdnnPercentile, rMSSDPercentile, hfpwrPercentile }, { rhrPercentile }] = await Promise.all([
        percentileHrvCalc(req, startDate, currentHRV),
        percentileRhrCalc(req, startDate, currentRHR)
    ])
    

    retArr = []
    if (currentHRV) {
        retArr = [...retArr,
        {
            label: "Heart Rate Variability (HF Power) Readiness",
            percentile: hfpwrPercentile,
            currentValue: currentHRV.HFPWR,
            createdAt: currentHRV.createdAt,
            id: currentHRV.id + '2',
            units: ''
        },
        {
            label: "Heart Rate Variability (rMSSD) Readiness",
            percentile: rMSSDPercentile,
            currentValue: currentHRV.rMSSD,
            createdAt: currentHRV.createdAt,
            id: currentHRV.id + '1',
            units: 'ms'
        },
        {
            label: "Heart Rate Variability (SDNN) Readiness",
            percentile: sdnnPercentile,
            currentValue: currentHRV.SDNN,
            createdAt: currentHRV.createdAt,
            id: currentHRV.id,
            units: 'ms'

        }]
    }
    if (currentRHR) {
        retArr = [
            {
                label: "Resting Heart Rate Readiness",
                percentile: rhrPercentile,
                currentValue: currentRHR.restingHeartRate,
                createdAt: currentRHR.createdAt,
                id: currentRHR.id,
                units: 'bpm'
            },
            ...retArr
        ]
    }

    res.status(200).json(retArr)


}))

// req for user id, startDate for time window, currentHRV for comparision
const percentileHrvCalc = async (req, startDate, currentHRV) => {
    return new Promise(async (resolve, reject) => {
        try {
            const [gtSDNN, ltSDNN, gtRMSSD, ltRMSSD, gtHFPWR, ltHFPWR] = await Promise.all([
                HRVReading.find({ user: req.user.id, SDNN: { $gt: currentHRV.SDNN }, createdAt: { $gte: startDate } }).count(),
                HRVReading.find({ user: req.user.id, SDNN: { $lte: currentHRV.SDNN }, createdAt: { $gte: startDate } }).count(),
                HRVReading.find({ user: req.user.id, rMSSD: { $gt: currentHRV.rMSSD }, createdAt: { $gte: startDate } }).count(),
                HRVReading.find({ user: req.user.id, rMSSD: { $lte: currentHRV.rMSSD }, createdAt: { $gte: startDate } }).count(),
                HRVReading.find({ user: req.user.id, HFPWR: { $gt: currentHRV.HFPWR }, createdAt: { $gte: startDate } }).count(),
                HRVReading.find({ user: req.user.id, HFPWR: { $lte: currentHRV.HFPWR }, createdAt: { $gte: startDate } }).count()
            ])
            sdnnPercentile = ltSDNN / (gtSDNN + ltSDNN) //higher is better
            rMSSDPercentile = ltRMSSD / (gtRMSSD + ltRMSSD) //higher is better
            hfpwrPercentile = ltHFPWR / (gtHFPWR + ltHFPWR) //higher is better

            resolve({
                ...currentHRV,
                sdnnPercentile: sdnnPercentile,
                rMSSDPercentile: rMSSDPercentile,
                hfpwrPercentile: hfpwrPercentile
            })
        }
        catch (e) {
            console.log(e)
            reject(e)
        }
    })
}

const percentileRhrCalc = async (req, startDate, currentRHR) => {
    return new Promise(async (resolve, reject) => {
        try {
            const [gtRHR, ltRHR] = await Promise.all([
                RHRReading.find({ user: req.user.id, restingHeartRate: { $gte: currentRHR.restingHeartRate }, createdAt: { $gte: startDate } }).count(),
                RHRReading.find({ user: req.user.id, restingHeartRate: { $lt: currentRHR.restingHeartRate }, createdAt: { $gte: startDate } }).count()
            ])
            
            rhrPercentile = gtRHR / (gtRHR + ltRHR) //lower is better
            resolve({...currentRHR, rhrPercentile: rhrPercentile })
        }
        catch (e) {
            console.log(e)
            reject(e)
        }
    })
}

module.exports = app