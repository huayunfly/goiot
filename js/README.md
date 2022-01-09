// Initialize a project, -y means creating a default package.json
mkdir example-project
cd example-project
npm init -y

// Install
npm install yargs --save
npm install redis --save
npm install pg --save

// Update
npm update -g package

// pg
dump to sqlfile: pgdump -U username -p port -f bkfile.pgsql dbname
restore: psql -U username -p port -d dbname -f bkfile.pgsql


