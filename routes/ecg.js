const express = require('express')
const app = express.Router()
const asyncHandler = require('../middleware/async')


app.post("/ecg", asyncHandler(async (req, res, next) => {
    //compute hrv stats e.g. SDNN, rMSSD, HFPWR, etc. from beat to beat data
    //console.log("ecg data", req.body.ecg)

    const rrIntervals = await computeRRIntervals(req.body.ecg)

    res.status(200).json({rs: rrIntervals})

    // return the obj even if it doesn't get stored (e.g. duplicate)
    //if (hrvObj) res.status(201).json(hrvObj)
    // store the computed obj in our readings database
    //if (hrvObj) await HRVReading.create({ ...hrvObj, user: req.user.id })

}))

const computeRRIntervals = async (ecg) => {
    const n = ecg.length

    //double differnce and square
    let d1 = ecg.map((e, i) => {
        const ei = e.voltageQuantity
        const ei1 = (i < n - 1) ? ecg[i+1].voltageQuantity : 0
        return ei1 - ei
    })
    d1 = d1.slice(0, n - 1)
    
    let d2 = d1.map((d1j, j) => {
        const d1j1 = (j < n - 2) ? d1[j+1] : 0
        return d1j1 - d1j
    })
    d2 = d2.slice(0, n - 2)
    d = d2.map((dj, j) => {return {...ecg[j], d: Math.pow(dj, 2)}}) 
    d = d.sort((a, b) => b.d - a.d)// sort in descending order
    const threshold = d[0].d * 0.03
    d = d.filter(e => e.d > threshold)

    let qrs = []
    
    // establish the qrs windows into their own array
    d.forEach(e => {
        const withinWindow = qrs.filter(r => Math.abs( e.timeSinceSeriesStart - r.timeSinceSeriesStart)/1000 < 75) //iz within 75ms
        if (withinWindow.length === 0) qrs.push(e) 
    });

    //find max and min voltage for each window and average them to get a baseline
    qrs = qrs.map(window => {
        return ecg.reduce((acc, cv) => {
            return {
                max: (cv.voltageQuantity > acc.max && Math.abs(cv.timeSinceSeriesStart - window.timeSinceSeriesStart)/1000 < 75) ? cv.voltageQuantity : acc.max,
                min: (cv.voltageQuantity < acc.min && Math.abs(cv.timeSinceSeriesStart - window.timeSinceSeriesStart)/1000 < 75) ? cv.voltageQuantity : acc.min
            }
        }, {max: 0, min: 0})
    }).map((cv, i) => { return {...qrs[i], baseline: (cv.max + cv.min)/2 }})//mean of max and min
    

    //use relative magnitude to find the R Peak voltage
    const rs = qrs.map(window => {
        return ecg.reduce((acc, cv) => {
            const relAmp = cv.voltageQuantity - window.baseline
            return (relAmp > acc.relAmp && Math.abs(cv.timeSinceSeriesStart - window.timeSinceSeriesStart)/1000 < 75) ? {...cv, relAmp: relAmp} : acc 
        }, {relAmp: 0})
    })

    console.log('rs', rs)
    return rs
}

module.exports = app