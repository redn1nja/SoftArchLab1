#[macro_use] extern crate rocket;

use rand::Rng;
use uuid::Uuid;
use rocket::serde::{Deserialize, json::Json};
use serde_json::json;

fn gen_addr() -> String {
    let mut rng = rand::thread_rng();
    std::env::args().nth(rng.gen_range(2..std::env::args().count())).unwrap().to_string()
}

#[derive(Deserialize)]
#[serde(crate="rocket::serde")]
struct Message {
    msg: String,
}

#[post("/", format="application/json", data = "<msg>")]
async fn post(msg: Json<Message>) -> String {
    let client = reqwest::Client::new();
    let uuid = Uuid::new_v4();
    let json = json!({
        "uuid": uuid.to_string(),
        "msg": msg.msg
    });
    let logging = gen_addr();
    println!("sending message {} to {} service", msg.msg, logging);
    let response = client.post(logging).json(&json).send().await.unwrap();
    response.text().await.unwrap()
}

#[get("/")]
async fn get() -> String {
    let client = reqwest::Client::new();
    let logging = gen_addr();
    println!("getting messages from {} service", logging);
    let message = std::env::args().nth(1).unwrap().to_string();
    let log_res = client.get(logging).send().await.unwrap();
    let msg_res = client.get(message).send().await.unwrap();
    let mut text = log_res.text().await.unwrap();
    let msg = msg_res.text().await.unwrap();
    text.push_str("\n");
    text.push_str(&msg);
    text

}

#[launch]
fn rocket() -> _ {
    rocket::build().configure(rocket::Config::figment().merge(("port", 3000))).mount("/", routes![post, get])
}