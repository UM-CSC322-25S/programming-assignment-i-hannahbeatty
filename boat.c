#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
NOTES FOR GRADER:
- Even though we assume the person entering data is rational, I still
  added error cases for the user attempting to add boats with invalid
  amounts owed, invalid letter chars for slip_number, etc. That is 
  why there are so many error cases in my code for parsing inputs
 
*/


// defines for constant values
#define TAG_LENGTH 7 // for max length of tag, 6 chars + null
#define SLIP_COST 12.5
#define LAND_COST 14
#define TRAILOR_COST 25
#define STORAGE_COST 11.20

// typedefs and convertors for enum (used in boat)
typedef enum {
  slip,
  land,
  trailor,
  storage,
  no_place
} PlaceType;

// converts a string to the enum
PlaceType string_to_place(char * place_str) {
  if (!strcasecmp(place_str,"slip"))
    return(slip);
  if (!strcasecmp(place_str,"land"))
    return(land);
  if (!strcasecmp(place_str,"trailor"))
    return(trailor);
  if (!strcasecmp(place_str,"storage"))
    return(storage);
  return(no_place);
}

// converts the enum to a string
char * place_to_string(PlaceType place) {
  switch (place) {
    case slip:
      return("slip");
    case land:
      return("land");
    case trailor:
      return("trailor");
    case storage:
      return("storage");
    case no_place:
      return("no_place");
    default:
      printf("How did I get here?\n");
      return "invalid";
  }
}

// union for containing location detail
typedef union {
  int slip;
  char letter;
  char tag[TAG_LENGTH];
  int storage_num;
} LocationDetail;

// struct defining boat data
// use typedef so we can use Boat instead of
// ^struct::Boat
typedef struct {
  char name[128]; // 127 chars + null
  float length;
  PlaceType location;
  LocationDetail detail;
  float owed;
} Boat;

// function I will use to free boat structs from []
void free_all_boats(Boat *boats[], int *boat_count) {
  for (int i = 0; i < *boat_count; i++) {
    free(boats[i]);
    boats[i] = NULL;
  }
  *boat_count = 0;
}

// comparator for qsort function
int compare_boats(const void *a, const void *b) {
  const Boat *boatA = *(const Boat **)a;
  const Boat *boatB = *(const Boat **)b;
  return strcasecmp(boatA->name, boatB->name);
}

// helper function for deleting boat - use binary search
// return -1 if boat isn't found, else return index of boats[]
int find_boat_index(int boat_count, Boat *boats[], const char * name) {
  // perform binary search
  int left = 0;
  int right = boat_count - 1;

  while (left <= right) {
    int mid = (left + right) / 2; // define midpoint
    int cmp = strcasecmp(boats[mid]->name, name); // keeps it case insensitive
    if (!cmp) { //if found
      return mid; // return index
    }
    else if (cmp < 0) { // look in right half
      left = mid + 1;
    }
    else { // look in left half
      right = mid - 1;
    }
  }
  return -1; // not found in []
}


// function for printing current inventory
void inventory(int boat_count, Boat * boats[]) {
  int counter = 0;
  while (counter < boat_count) {
    Boat * b = boats[counter]; // current boat ptr
    printf("%-22s%4.0f'%4s", b->name, b->length, "");
    printf("%-8s", place_to_string(b->location));

    // switch case for location, detail formatting
    switch(b->location) {
      case slip:
        printf("  # %2d", b->detail.slip);
        break;
      case land:
        printf("     %c", b->detail.letter);
        break;
      case trailor:
        printf("%6s", b->detail.tag);
        break;
      case storage:
         printf("  # %2d", b->detail.storage_num);
         break;
      default:
        printf("???");
        break;
    }
    printf("   Owes $%7.2f\n", b->owed);
    counter++; // go until condition is met
  }
}

// function for updating amounts owed each month
void monthly_charges(int *boat_count, Boat * boats[]) {
  int count = 0;
  // traverse through whole array:
  while (count < *boat_count) {
    // switch by location to charge proper amount
    switch(boats[count]->location) {
      case slip:
        boats[count]->owed += boats[count]->length * SLIP_COST;
        break;
      case land:
        boats[count]->owed += boats[count]->length * LAND_COST;
        break;
      case trailor:
        boats[count]->owed += boats[count]->length * TRAILOR_COST;
        break;
      case storage:
        boats[count]->owed += boats[count]->length * STORAGE_COST;
        break;
      default: // shouldn't get here, this is checked for earlier
        printf("Something's wrong.\n");
        break;
    }
    count++;
  }
}


// helper function used for reading csv and adding boats
// NOTE - all error cases returned as integers, handled in UX
int parse_and_create_boat(char line[], Boat **new_boat_out) {
  char name[128], place_str[16], detail[32];
  float length, owed;

  // if five arguments are not properly passed in one line
  if (sscanf(line, "%127[^,],%f,%15[^,],%31[^,],%f", 
      name, &length, place_str, detail, &owed) != 5)
    return -1; 
  
  // test for malloc error
  Boat *new_boat = malloc(sizeof(Boat));
  if (!new_boat) 
    return -2;

  // add boat name
  strncpy(new_boat->name, name, sizeof(new_boat->name) - 1);
  new_boat->name[sizeof(new_boat->name) - 1] = '\0';
  // test if length is valid 
  if (length > 100 || length < 0) {
    free(new_boat);
    return -3;
  }
  // add length and location attributes
  new_boat->length = length;
  new_boat->location = string_to_place(place_str);
  // check if location is valid
  if (new_boat->location == no_place) { // improper location
    free(new_boat);
    return -4;
  }
  // test if owed is impossible
  if (owed < 0) {
    free(new_boat);
    return -5;
  }
  // add owed
  new_boat->owed = owed;

  // switch by location to test and add details
  switch (new_boat->location) {
    case slip: {
      char *endptr;
      long slip_num = strtol(detail, &endptr, 10);
      if (*endptr != '\0' || slip_num < 1 || slip_num > 85) {
        free(new_boat);
        return -6; 
      }
      // convert to int and add detail.slip attribute
      new_boat->detail.slip = (int)slip_num;
      break;
    }
    case land: {
      if (strlen(detail) != 1 || detail[0] < 'A' || detail[0] > 'Z') {
        free(new_boat);
        return -7;
      }
      new_boat->detail.letter = detail[0];
      break;
    }
    case trailor: {
      size_t len = strlen(detail);
      if (len == 0 || len >= TAG_LENGTH) {
        free(new_boat);
        return -8;
      }
      strncpy(new_boat->detail.tag, detail, TAG_LENGTH - 1);
      new_boat->detail.tag[TAG_LENGTH - 1] = '\0';
      break;
    }
    case storage: {
      char *endptr2;
      long storage_num = strtol(detail, &endptr2, 10);
      if (*endptr2 != '\0' || storage_num < 1 || storage_num > 50) {
        free(new_boat);
        return -9;
      }
      new_boat->detail.storage_num = (int)storage_num;
      break;
    }
    default: {
      free(new_boat);
      return -10; // shouldnt happen, just in case
    }
  }
  
  *new_boat_out = new_boat;
  return 0; // success

}


int add_boat(Boat *boats[], int *boat_count, char line[]) {
  if (*boat_count == 120)
    return 0; // return error flag
  Boat *new_boat;
  int result = parse_and_create_boat(line, &new_boat);
  if (result != 0)
    return result;
  if (find_boat_index(*boat_count, boats, new_boat->name) != -1) {
    free(new_boat);
    return -11; // duplicate name
  }
  boats[*boat_count] = new_boat;
  (*boat_count)++;
  qsort(boats, *boat_count, sizeof(Boat *), compare_boats);
  return 1; // success
}


// return -1 if boat not found, else return 1
int delete_boat(int *boat_count, Boat *boats[], const char * name) {
  int index = find_boat_index(*boat_count, boats, name);
  if (index == -1) {
    return -1;
  }
  // free boat at index
  free(boats[index]); 

  // shift all boats to left over to fill gap
  for (int i = index; i < *boat_count - 1; i++) {
    boats[i] = boats[i + 1];
  }
  boats[*boat_count - 1] = NULL; // not technically necessary, clear buffer
  (*boat_count)--;
  return 1;
}

int make_payment(int *boat_count, Boat *boats[], int amount, int index) {
// in order to get here, we have already checked that boat exists!
  if (amount <= 0) {
    return 0; 
  }
  if (amount > boats[index] -> owed) {
    return -1;
  }
  boats[index]->owed -= amount;
  return 1;
}

void read_csv(const char *filename, Boat *boats[], int *boat_count) {
  FILE * file = fopen(filename, "r"); // read only for now
  if (!file) {
    perror("Issue opening file.");
    exit(EXIT_FAILURE);
  }
  
  // add logic for stopping at 120 and indicating more than 120 boats were in csv
  char line[256];
  while (fgets(line, sizeof(line), file)) {
    if (*boat_count >= 120) {
      fprintf(stderr, "Marina is full.");
      return;
    }
    line[strcspn(line, "\n")] = '\0';
    Boat *new_boat;
    if (parse_and_create_boat(line, &new_boat) != 0) {
      fprintf(stderr, "Skipping malformed line: %s\n", line);
      continue;
    }
    if (find_boat_index(*boat_count, boats, new_boat->name) != -1) {
      free(new_boat);
      fprintf(stderr, "Cannot add duplicate boat names.");
      continue;
    }     
    boats[*boat_count] = new_boat;
    (*boat_count)++;
  }
  fclose(file);
  qsort(boats, *boat_count, sizeof(Boat *), compare_boats);
}

void menu_handling(Boat *boats[], int *boat_count) {
  printf("Welcome to the Boat Management System\n");
  printf("-------------------------------------\n");
  int current = 1;
  char choice;
  while (current) {
    printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
    
    if (scanf(" %c", &choice) != 1) {
      // Clear the input buffer first before continuing
      while (getchar() != '\n' && getchar() != EOF); // clear
      printf("ERROR - Improper input. Please try again.\n");
      continue;
    }

    while (getchar() != '\n'); // flush leftover newline before fgets
    
    // print the INVENTORY
    if (choice == 'I' || choice == 'i') { // case insensitive
      inventory(*boat_count, boats);
      continue;
    }

    // ADD a boat tp inventory
    else if (choice == 'A' || choice == 'a') { // case insensitive
      char dummy[256];
      printf("Please enter the boat data in CSV format: ");
      if (fgets(dummy, sizeof(dummy), stdin) == NULL) {
        printf("ERROR: Failed to read input. Boat not added.\n");
        continue;
      }
      dummy[strcspn(dummy, "\n")] = '\0'; // Strip newline
      
      int result = add_boat(boats, boat_count, dummy); // check if boat_count is passed right
      // below are error messages associated with each error code from adding boats
      if (result == 0) {
        printf("ERROR: 120 boats are already in the marina."
        " No new boats can be added.\n");
        continue;
      }
      else if (result == -1) {
        printf("ERROR: Please enter valid comma-separated values. "
        "Boat not added.\n");
        continue;
      }
      else if (result == -2) {
        printf("ERROR: Issue allocating boat.\n");
        continue;
      }
      else if (result == -3) {
        printf("ERROR: User attempted to add a boat with an invalid length."
        " Boat not added.\n");
        continue;
      }
      else if (result == -4) {
        printf("ERROR: User attempted to add a boat with an invalid location type. "
        "Boat not added.\n");
        continue;
      }

      else if (result == -5) {
        printf("ERROR: User attempted to enter an invalid amount owed, "
        "less than 0. Boat not added.\n");
        continue;
      }
      
      else if (result == -6) {
        printf("ERROR: user entered an invalid slip number value. Boat not added.\n");
        continue;
      }
      
      else if (result == -7) {
        printf("ERROR: user entered an invalid bay letter value. Boat not added.\n");
        continue;
      }

      else if (result == -8) {
        printf("ERROR: user entered an invalid trailor tag. Boat not added.\n");
        continue;
      }
 
      else if (result == -9) {
        printf("ERROR: user entered an invalid storage number. Boat not added.\n");
        continue;
      }

      else if (result == -11) {
        printf("ERROR: A boat with this name already exists. Boat not added.\n");
        continue;
      }

      else if (result == 1) {
        printf("Boat successfully added!\n");
        continue;
      }
      
      else {
        printf("Something's wrong!\n");
        free_all_boats(boats, boat_count);
        exit(EXIT_FAILURE);
      }
    }
    
    // REMOVE boat
    else if (choice == 'R' || choice == 'r') { // case insensitive.
      char name[128];
      printf("Please enter the name of the boat to remove: ");
      while(1) {
        if (fgets(name, sizeof(name), stdin) == NULL) {
          printf("ERROR: Please try again with a valid name. \n");
          printf("Waiting for valid name... ");
          continue;
        }
        name[strcspn(name, "\n")] = '\0';
        break;
      }
      int result = delete_boat(boat_count, boats, name);
      if (result == 1) {
        printf("Successfully deleted %s from database!\n", name);
        continue;
      }
      else if (result == -1) {
        printf("ERROR: There is no boat named %s in the database."
        " No boats deleted.\n", name);
        continue;
      }

      else {
        printf("ERROR: Something went wrong.\n");
        free_all_boats(boats, boat_count);
        exit(EXIT_FAILURE);
      }
    }

    // make PAYMENT
    else if (choice == 'P' || choice == 'p') {
      char name[128];
      float amount = 0;
      printf("Please enter the boat name: ");
      while (1) {
        if (fgets(name, sizeof(name), stdin) == NULL) {
          printf("ERROR: Please try again with a valid name. \n");
          printf("Waiting for valid name... ");
          continue;
        }
        name[strcspn(name, "\n")] = '\0';
        break;
      }
      int index = find_boat_index(*boat_count, boats, name);
      if (index == -1) {
        printf("ERROR: There is no boat named %s in the database. No payment made.\n", name);
        continue;
      }
      printf("Please enter the amount to be paid: ");
      if (scanf("%f", &amount) != 1) {
        // clear buffer on failure
        while (getchar() != '\n' && getchar() != EOF);
        printf("ERROR: Invalid payment amount.\n");
        continue;
      }
      // clear buffer after successful read
      while (getchar() != '\n' && getchar() != EOF);

      int result = make_payment(boat_count, boats, amount, index);
      if (result == 0) {
        printf("ERROR: You cannot make a negative payment. No payment made.\n");
        continue;
      }
      else if (result == -1) {
        printf("That is more than the amount owed, $%.2f. No payment made.\n", boats[index]->owed);
        continue;
      }
      else {
        printf("Amount $%.2f paid.\n", amount);
      }
    }

    else if (choice == 'M' || choice == 'm') {
      monthly_charges(boat_count, boats);
      printf("Amounts owed have been updated for the new month. \n");
      continue;
    }

    else if (choice == 'X' || choice == 'x') {
      printf("Now exiting the Boat Management System.\n");
      return;
      // exit, memory is freed in main 
    }

    else {
      // handle the issue here
      printf("ERROR: User did not enter a valid input from the menu.\n");
      continue;
    }
  }
} 

void update_csv(const char *filename, Boat *boats[], int *boat_count) {
  // open file we originally read from
  FILE *file = fopen(filename, "w"); // CLEARS ORIGINAL FILE
  if (!file) {
    perror("Error: could not open your original file to save changes.");
    free_all_boats(boats, boat_count);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < *boat_count; i++) {
    Boat *b = boats[i];
    fprintf(file, "%s,%.0f,%s,", b->name, b->length, place_to_string(b->location));

    switch (b->location) {
      case slip:
        fprintf(file, "%d,", b->detail.slip);
        break;
      case land:
        fprintf(file, "%c,", b->detail.letter);
        break;
      case trailor:
        fprintf(file, "%s,", b->detail.tag);
        break;
      case storage:
        fprintf(file, "%d,", b->detail.storage_num);
        break;
      default: // shouldn't EVER happen, taken care of earlier
        fprintf(stderr, "ERROR: Invalid location type for boat %s\n", b->name);
        fclose(file);
        free_all_boats(boats, boat_count);
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%.2f\n", b->owed);
  }
  if (fclose(file) != 0) { // test for closing error
    perror("ERROR: Failed to close file");
    free_all_boats(boats, boat_count); // free data
    exit(EXIT_FAILURE); // exit fail
  }
}


int main(int argc, char *argv[]) { // takes in argument
  if (argc != 2) {
    printf("ERROR: improper number of arguments. Please try again.\n");
    return EXIT_FAILURE;
  }

  Boat *boats[120] = {NULL}; // initialize empty array of boat ptrs
  int boat_count = 0; // save number of boats
  read_csv(argv[1], boats, &boat_count); // read from csv
  menu_handling(boats, &boat_count); // user interaction
  update_csv(argv[1], boats, &boat_count); // save to original csv
  free_all_boats(boats, &boat_count); // free the boat allocated structs
}



