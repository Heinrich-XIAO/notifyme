const currentHour = new Date().getHours();
const currentMinute = new Date().getMinutes();

if (currentMinute % 5 === 0) {
	console.log("This is the time");
	console.log(`It is currently ${currentHour.toString().padStart(2, '0')}:${currentMinute.toString().padStart(2, '0')}`);
}
