#################################
# 1044731 KOYTZAGKOYLAKI STAMATIA
# 1072479 TSIOUTSIAS NIKOLAOS
#################################

#!/bin/bash

while true
do
  echo " "
  echo "Παρακαλώ επιλέξτε λειτουργία"
  echo " "
  echo "[1] Επιλογή αρχείου επιχειρήσεων"
  echo "[2] Προβολή στοιχείου επιχείρησης"
  echo "[3] Aλλαγή στοιχείου επιχείρησης"
  echo "[4] Προβολή αρχείου"
  echo "[5] Αποθήκευση αρχείου"
  echo "[6] Έξοδος"
  echo " "
  read -r choice

  if [ "$choice" == "1" ]; then
    echo "Επιλέχθηκε η λειτουργία [1] Επιλογή αρχείου επιχειρήσεων"
    echo "Παρακαλώ επιλέξτε αρχείο"
    read -r file 

    if [ -z "$file" ]; then 
      echo "Δεν βρέθηκε αρχείο $file. Το αρχείο με όνομα Businesses.csv επιλέχθηκε αυτόματα."
      file="Businesses.csv"
    else
      echo "Το αρχείο με όνομα $file επιλέχθηκε."
    fi

  elif [ "$choice" == "2" ]; then
    echo "Επιλέχθηκε η λειτουργία [2] Προβολή στοιχείου επιχείρησης"
    echo "Παρακαλώ επιλέξτε αρχείο"
    read -r file 

    if [ -z "$file" ]; then 
      echo "Δεν βρέθηκε αρχείο $file. Το αρχείο με όνομα Businesses.csv επιλέχθηκε αυτόματα."
      file="Businesses.csv"
    else
      echo "Το αρχείο με όνομα $file επιλέχθηκε."
    fi
    
    echo "Εισαγωγή κωδικού επιχείρησης"
    read -r code

    search=$(awk -F',' -v pass="$code" '$1 == pass {print}' "$file")

    if [ -z "$search" ]; then 
      echo "Δεν υπάρχει επιχείρηση με κωδικό $code ."
    else     
      IFS=',' read -r code name address city post_id longitude latitude <<< "$search"

      echo "Προβολή των στοιχείων της επιχείρησης με κωδικό $code."
      echo "κωδικός: $code"
      echo "όνομα: $name"
      echo "οδός: $address"
      echo "πόλη: $city"
      echo "ταχυδρομικός κώδικας: $post_id"
      echo "γεωγραφικό μήκος: $longitude"
      echo "γεωγραφικό πλάτος: $latitude"
    fi

  elif [ "$choice" == "3" ]; then
    echo "Επιλέχθηκε η λειτουργία [3] Αλλαγή στοιχείου επιχείρησης"
    echo "Παρακαλώ επιλέξτε αρχείο"
    read -r file 

    if [ -z "$file" ]; then 
      echo "Δεν βρέθηκε αρχείο $file. Το αρχείο με όνομα Businesses.csv επιλέχθηκε αυτόματα."
      file="Businesses.csv"
    else
      echo "Το αρχείο με όνομα $file επιλέχθηκε."
    fi

    echo "Δώστε τον κωδικό επιχείρησης"
    read -r code

    search=$(awk -F',' -v pass="$code" '$1 == pass {print}' "$file")

    if [ -z "$search" ]; then 
      echo "Δεν υπάρχει επιχείρηση με κωδικό $code."
    else 
      echo "Επιλογή στοιχείου προς αλλαγή:"
      echo "1. κωδικός"
      echo "2. όνομα"
      echo "3. οδός"
      echo "4. πόλη"
      echo "5. ταχυδρομικός κώδικας"
      echo "6. γεωγραφικό μήκος"
      echo "7. γεωγραφικό πλάτος"

      read -r option
     
      temp_file=$(mktemp)
      trap 'rm -f "$temp_file"' EXIT
      cp "$file" "$temp_file"  
      
      case $option in
      1)
        echo "Καταχώρηση νέου κωδικού:"
        read -r n_code
        awk -v pass="$code" -v n_code="$n_code" 'BEGIN{FS=OFS=","} $1 == pass {$1=n_code}{print}'"$temp_file" > "$file.tmp" && mv "$file.tmp" "$file"
      ;;
  
      2)
        echo "Καταχώρηση νέου ονόματος:"
        read -r n_name
        awk -v pass="$code" -v n_name="$n_name" 'BEGIN{FS=OFS=","} $1 == pass {$2=n_name}{print}'"$temp_file" > "$file.tmp" && mv "$file.tmp" "$file"
      ;;
  
      3)
        echo "Καταχώρηση νέας οδού:"
        read -r n_address
        awk -v pass="$code" -v n_address="$n_address" 'BEGIN{FS=OFS=","} $1 == pass {$3=n_address}{print}'"$temp_file" > "$file.tmp" && mv "$file.tmp" "$file"
      ;;
  
      4)
        echo "Καταχώρηση νέας πόλης:"
        read -r n_city
        awk -v pass="$code" -v n_city="$n_city" 'BEGIN{FS=OFS=","} $1 == pass {$4=n_city}{print}'"$temp_file" > "$file.tmp" && mv "$file.tmp" "$file"
      ;;
  
      5)
        echo "Καταχώρηση νέου ταχυδρομικού κώδικα:"
        read -r n_postalcode
        awk -v pass="$code" -v n_postalcode="$n_postalcode" 'BEGIN{FS=OFS=","} $1 == pass {$5=n_postalcode}{print}'"$temp_file" > "$file.tmp" && mv "$file.tmp" "$file"
      ;;
  
      6)
        echo "Καταχώρηση νέου γεωγραφικού μήκους:"
        read -r n_longitude
        awk -v pass="$code" -v n_longitude="$n_longitude" 'BEGIN{FS=OFS=","} $1 == pass {$6=n_longitude}{print}'"$temp_file" > "$file.tmp" && mv "$file.tmp" "$file"
      ;;
  
      7)
        echo "Καταχώρηση νέου γεωγραφικού πλάτους:"
        read -r n_latitude
        awk -v pass="$code" -v n_latitude="$n_latitude" 'BEGIN{FS=OFS=","} $1 == pass {$7=n_latitude}{print}'"$temp_file" > "$file.tmp" && mv "$file.tmp" "$file"
      ;;
  
      *)
        echo "Η επιλογή αυτή δεν υπάρχει στην λίστα"
      ;;
      esac

      # Εμφάνιση παλιών και νέων στοιχείων
      IFS=',' read -r code name address city postal_code longitude latitude <<< "$search"

      echo "Το στοιχείο που επιλέξατε άλλαξε."
      echo " "
      echo "Προβολή παλιών και νέων στοιχείων της επιχείρησης με κωδικό $code."
      echo " "
      echo "Παλιά Στοιχεία:"
      echo "Κωδικός: $code"
      echo "Όνομα: $name"
      echo "Οδός: $address"
      echo "Πόλη: $city"
      echo "Ταχυδρομικός Κώδικας: $postal_code"
      echo "Γεωγραφικό Μήκος: $longitude"
      echo "Γεωγραφικό Πλάτος: $latitude"
      echo " "
      echo "Νέα στοιχεία "
      case $option in
             
        1)
          echo "Κωδικός: $n_code"
          echo "Όνομα: $name"
          echo "Οδός: $address"
          echo "Πόλη: $city"
          echo "Ταχυδρομικός Κώδικας: $postal_code"
          echo "Γεωγραφικό Μήκος: $longitude"
          echo "Γεωγραφικό Πλάτος: $latitude"
        ;;
        2)
          echo "Κωδικός: $code"
          echo "Όνομα: $n_name"
          echo "Οδός: $address"
          echo "Πόλη: $city"
          echo "Ταχυδρομικός Κώδικας: $postal_code"
          echo "Γεωγραφικό Μήκος: $longitude"
          echo "Γεωγραφικό Πλάτος: $latitude"
        ;;    
        3)
          echo "Κωδικός: $code"
          echo "Όνομα: $name"
          echo "Οδός: $n_address"
          echo "Πόλη: $city"
          echo "Ταχυδρομικός Κώδικας: $postal_code"
          echo "Γεωγραφικό Μήκος: $longitude"
          echo "Γεωγραφικό Πλάτος: $latitude"
        ;;    
        4)
          echo "Κωδικός: $code"
          echo "Όνομα: $name"
          echo "Οδός: $address"
          echo "Πόλη: $n_city"
          echo "Ταχυδρομικός Κώδικας: $postal_code"
          echo "Γεωγραφικό Μήκος: $longitude"
          echo "Γεωγραφικό Πλάτος: $latitude"
   		  ;;
        5)
          echo "Κωδικός: $code"
          echo "Όνομα: $name"
          echo "Οδός: $address"
          echo "Πόλη: $city"
          echo "Ταχυδρομικός Κώδικας: $n_postalcode"
          echo "Γεωγραφικό Μήκος: $longitude"
          echo "Γεωγραφικό Πλάτος: $latitude"
        ;;
        6)
          echo "Κωδικός: $code"
          echo "Όνομα: $name"
          echo "Οδός: $address"
          echo "Πόλη: $city"
          echo "Ταχυδρομικός Κώδικας: $postal_code"
          echo "Γεωγραφικό Μήκος: $n_longitude"
          echo "Γεωγραφικό Πλάτος: $latitude"
 		    ;;
        7)
          echo "Κωδικός: $code"
          echo "Όνομα: $name"
          echo "Οδός: $address"
          echo "Πόλη: $city"
          echo "Ταχυδρομικός Κώδικας: $postal_code"
          echo "Γεωγραφικό Μήκος: $longitude"
          echo "Γεωγραφικό Πλάτος: $n_latitude"
		    ;;
        *) echo "Δεν ανανεώθηκε κάποιο στοιχείο"
        ;;
      esac

    fi

  elif [ "$choice" == "4" ]; then
    echo "Επιλέχθηκε η λειτουργία [4] Προβολή αρχείου"
    echo "Για έξοδο από το αρχείο πατήστε q, για να συνεχίσετε πατήστε space"
    echo "Παρακαλώ επιλέξτε αρχείο"
    read -r file 
    
    if [ -z "$file" ]; then 
      echo "Δεν βρέθηκε αρχείο $file. Το αρχείο με όνομα Businesses.csv επιλέχθηκε αυτόματα."
      file="Businesses.csv"
    else
      echo "Το αρχείο με όνομα $file επιλέχθηκε."
    fi
    
    less "$file"

  elif [ "$choice" == "5" ]; then
    echo "Επιλέχθηκε η λειτουργία [5] Αποθήκευση αρχείου "
    echo "Παρακαλώ επιλέξτε αρχείο"
    read -r file 

    if [ -z "$file" ]; then 
      echo "Δεν δόθηκε αρχείο $file με πελατολόγιο."
      exit 0
    else
      echo "Το αρχείο με όνομα $file επιλέχθηκε."
    fi

    echo "Παρακαλώ εισάγετε το path που θα αποθηκευτεί το πελατολόγιο του αρχείου $file"
    read -r file_path 

    if [ -z "$file_path" ]; then 
      echo "Δεν δόθηκε path. Η αποθήκευση πελατολογίου στο αρχείο Businesses.csv πραγματοποιήθηκε αυτόματα"
      file_path="Businesses.csv"      
    else
      echo "Αποθήκευση πελατολογίου στο αρχείο "$file_path"."
    fi
    touch "$file_path"
    cp "$file" "$file_path"
  elif [ "$choice" == "6" ] ; then
    echo "Επιλέχθηκε [6] Έξοδος"
    exit 0
      
  else
    echo "Η λειτουργία που ζητήσατε δεν βρέθηκε"
  fi

done


