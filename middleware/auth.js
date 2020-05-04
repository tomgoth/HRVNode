const jwt = require('jsonwebtoken');

// middleware is just a function that has access to the request and response cycle and object
    // i want to check if there is a token in the header
    // next is just saying move onto the next piece of middleware
    // x-auth-token is the key

module.exports = function(req, res, next) {
    // Get token from header
    const token = req.header('x-auth-token')

    // Check if not token
    if(!token) {
        return res.status(401).json({ msg: 'No token, authorization denied' })
    }

    // If token, verify it
    try {
        const decoded = jwt.verify(token, process.env.jwtSecret)
        // once it's verified, the object / payload is going to be put into decoded and we want to take the user out and assign the user to request object
        req.user = decoded.user;
        next()
        
    } catch (err) {
        res.status(401).json({ msg: 'Token is not valid' })
    }
}