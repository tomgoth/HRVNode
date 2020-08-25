const express = require('express'); //(2) -->routes in file
const router = express.Router();
const { check, validationResult } = require('express-validator');
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const auth = require('../middleware/auth');
const User = require('../schemas/User');

// @route   GET api/auth
// @desc    Get logged in user
// @access  Private
router.get('/', auth, async (req, res) => { //(3)
    
    try {
        // select is making sure you dont return the passowrd from request
        // accessing the user id via the payload hash
        const user = await User.findById(req.user.id).select('-password')
        res.json(user)
    } catch (err) {
        console.error(err.message)
        res.status(500).json({ msg: 'Server Error' })
    }

})

// @route   POST api/auth
// @desc    Auth user and get token
// @access  Public
router.post('/',  [ //(4) --> users
    check('email', 'Please include a valid email').isEmail(),
    check('password', 'Password is required').exists()
], async (req, res) => {

    const errors = validationResult(req)

    if(!errors.isEmpty()) { // if there are any errors then send a response
        return res.status(400).json({ errors: errors.array() })
    }

    req.body.email = req.body.email.toLowerCase()
    const { email, password } = req.body

    try {
        let user = await User.findOne({ email }) // findOne returns a promise

        if(!user) { // if no user with that email 
            return res.status(400).json({ msg: 'Invalid credentials' })
        }

        const isMatch = await bcrypt.compare(password, user.password)

        if(!isMatch) { // if the password doesnt match our hash
            return res.status(400).json({msg: 'Invalid credentials' })
        }

        // if it does match then we wnt to send the token -- just like in register route
        // payload is the object I want to send in the token 
        const payload = {
            user: {
                id: user.id
            }
        }

        jwt.sign(payload, process.env.jwtSecret, { expiresIn: 31536000 }, (err, token) => {
            if (err) {
                throw err
            }

            res.json({ token })
        })

    } catch (err) {
        console.error(err.message)
        res.status(500).json({ msg: 'Server Error' })
    }
})

module.exports = router;