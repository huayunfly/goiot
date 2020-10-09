/// npm taobao mirror: npm install -gd express --registry=http://registry.npm.taobao.org
/// Or setting the global: npm config set registry http://registry.npm.taobao.org

const read_file = require('fs').readFile;
const yargs = require('yargs');
const argv = yargs.demandOption('f').nargs('f', 1).describe('f', 'JSON file to parse').argv;
const file = argv.f;

read_file(argv.f, (err, data_buffer) => {
    if (err)
    {
        console.error(`ERROR: ${err.message}`);
        process.exit(1);
    }
    else
    {
        const value = JSON.parse(data_buffer.toString());
        console.log(JSON.stringify(value));
    }
});

