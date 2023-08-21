// Initialize a project, -y means creating a default package.json
mkdir example-project
cd example-project
npm init -y

// npm mirror server
npm config set registry https://registry.npmmirror.com/

// Install
npm install yargs --save
npm install redis --save
npm install pg --save
npm install fastify --save
npm install node-fetch --save

// Update
npm update -g package

// pg
dump to sqlfile: 
	pg_dump.exe -U username -p port -f bkfile.pgsql dbname
restore: 
	psql.exe -U username -p port -d dbname -f bkfile.pgsql


