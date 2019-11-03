module.exports = {
    "env": {
        "browser": true,
        "es6": true,
        "jquery": true
    },
    "extends": "eslint:recommended",
    "globals": {
        "Atomics": "readonly",
        "SharedArrayBuffer": "readonly"
    },
    "parserOptions": {
        "ecmaVersion": 2018,
        "sourceType": "script"
    },
    "rules": {
      "curly": [
        2,
        "all"
      ],
      "operator-linebreak": [
        2,
        "after"
      ],
      "camelcase": [
        2,
        {
          "properties": "never"
        }
      ],
      "max-len": [
        2,
       80
      ],
      "indent": [
        2,
        2,
        {
          "SwitchCase": 1
        }
      ],
      "quotes": [
        2,
        "single"
      ],
      "no-multi-str": 2,
        "no-mixed-spaces-and-tabs": 2,
      "no-trailing-spaces": 2,
      "space-unary-ops": [
        2,
       {
          "nonwords": false,
          "overrides": {}
        }
        ],
      "one-var": [
        2,
        "never"
      ],
     "keyword-spacing": [
        2,
        {}
      ],
      "space-infix-ops": 2,
        "space-before-blocks": [
        2,
        "always"
      ],
     "eol-last": 2,
      "space-before-function-paren": [
        2,
        "never"
      ],
      "array-bracket-spacing": [
        2,
        "never",
        {
          "singleValue": true
        }
      ],
      "space-in-parens": [
        2,
        "never"
      ],
      "no-multiple-empty-lines": 2
    }
};
