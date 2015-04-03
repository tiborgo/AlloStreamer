#pragma strict
var CameraObject : Transform;
static var helicopter : Transform;
private var look : boolean = false;
private var angleGaffer : float;

private var P1 : float;
private var P2 : float;
private var P3 : float;


function Start() {
	yield WaitForSeconds(1);
	P3 = 0.0;
	look = true;
}
function Update () {
	if(look == true) {
		if(transform.position.y > -1.0) {
		P1 += Time.deltaTime/15;
		P2 = 0.0;
		angleGaffer = Mathf.Lerp(angleGaffer, 20.0, P1);
		} else {
		P2 += Time.deltaTime/15;
		P1 = 0.0;
		angleGaffer = Mathf.Lerp(angleGaffer, 0.0, P2);
		}
		P3 += Time.deltaTime;
		transform.rotation = Quaternion.Euler(0,P3*5, Mathf.Sin(P3/2.5) * angleGaffer);
		CameraObject.transform.LookAt(helicopter.transform.position);
		transform.position.y = Mathf.Lerp(transform.position.y, helicopter.transform.position.y+3, P3/10);
	}
	
}