
struct auth_response {
  unsigned int auth_result: 0; // 0 or 1 for true or false
}


struct auth_response check_auth() {
  struct auth_response response;
  response.auth_result = 1; // change to 0 for false
  return response;
}

// example function for you
void control(auth_response auth) {

}

void setup() {

}

void loop() {
  struct auth_response response = check_auth();
  control(response);

}
