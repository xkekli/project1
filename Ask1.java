/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package aski1_big_data;

import java.io.*;
import java.util.*;

/**
 *
 * @author Χρήστος
 */
public class Ask1 {
    
    private static int min_support = 6500;
    
    private void echo_results_to_file(String filename, TreeMap<String, Integer> data_object){
        try {
            File file = new File(filename);
            if (!file.exists()) {
                file.createNewFile();
            }
            FileWriter fw = new FileWriter(file.getAbsoluteFile());
            BufferedWriter bw = new BufferedWriter(fw);
            for (Map.Entry<String, Integer> entry : data_object.entrySet()) {
                String key = entry.getKey();
                Integer value = entry.getValue();
                bw.write(key + " => " + value + "\n");
            }
            bw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        BufferedReader br = null;
        String basket, pair_id, triple_id;
        String[] different_items;
        int baskets_read = 0, i, j, count;
        TreeMap<String, Integer> product_counters = new TreeMap<String, Integer>();
        TreeMap<String, Integer> product_pair_counters = new TreeMap<String, Integer>();
        TreeMap<String, Integer> product_triple_counters = new TreeMap<String, Integer>();
        Integer t_counter, t_counter2, t_counter3;
        Ask1 ask1_object = new Ask1();
        System.out.println("Pass 1 starts");
        try {
            br = new BufferedReader(new FileReader("baskets.txt"));
            while ((basket = br.readLine()) != null) {
                different_items = basket.split(" ");
                for (String item : different_items) {
                    t_counter = product_counters.get(item);
                    if (t_counter == null) {
                        product_counters.put(item, 1);
                    } else {
                        product_counters.put(item, t_counter + 1);

                    }
                }
                baskets_read++;
                //System.out.println(basket);
            }
        } catch (IOException e) {
            System.out.println("Exception 1 thrown  :" + e);
        } finally {
            try {
                br.close();
            } catch (IOException e) {
                System.out.println("Exception 2 thrown  :" + e);
            }
        }
        System.out.println(baskets_read + " baskets read");
        //  System.out.println(product_counters);
        System.out.println(product_counters.size() + " different_products");
        System.out.println("Pass 1 ends");
        System.out.println("Pass 2 starts");
        baskets_read = 0;
        try {
            br = new BufferedReader(new FileReader("baskets.txt"));
            while ((basket = br.readLine()) != null) {
                different_items = basket.split(" ");
                Arrays.sort(different_items);
                for (i = 0, count = different_items.length; i < count - 1; i++) {
                    for (j = i + 1; j < count; j++) {
                        pair_id = different_items[i] + '-' + different_items[j];
                        t_counter = product_pair_counters.get(pair_id);
                        if (t_counter == null) {
                            t_counter2 = product_counters.get(different_items[i]);
                            t_counter3 = product_counters.get(different_items[j]);
                            if (t_counter2 != null
                                && t_counter2 >= min_support
                                && t_counter3 != null
                                && t_counter3 >= min_support
                                && !different_items[i].equals(different_items[j])) {
                                product_pair_counters.put(pair_id, 1);
                            }
                        } else {
                            product_pair_counters.put(pair_id, t_counter + 1);
                        }
                    }
                }
                baskets_read++;
            }
        } catch (IOException e) {
            System.out.println("Exception 3 thrown  :" + e);
        } finally {
            try {
                br.close();
            } catch (IOException e) {
                System.out.println("Exception 4 thrown  :" + e);
            }
        }
        System.out.println(baskets_read + " baskets read");
        ask1_object.echo_results_to_file("pass2_counters.txt", product_pair_counters);
        System.out.println(product_pair_counters.size() + " different_pair_products");
        System.out.println("Pass 2 ends");
        System.out.println("Pass 3 starts");
        baskets_read = 0;
        try {
            br = new BufferedReader(new FileReader("baskets.txt"));
            while ((basket = br.readLine()) != null) {
                different_items = basket.split(" ");
                Arrays.sort(different_items);
                for (i = 0, count = different_items.length; i < count - 2; i++) {
                    pair_id = different_items[i] + '-' + different_items[i + 1];
                    for (j = i + 2; j < count; j++) {
                        triple_id = pair_id + '-' + different_items[j];
                        t_counter = product_triple_counters.get(triple_id);
                        if (t_counter == null) {
                            t_counter2 = product_pair_counters.get(pair_id);
                            t_counter3 = product_counters.get(different_items[j]);
                            if (t_counter2 != null
                                && t_counter2 >= min_support
                                && t_counter3 != null
                                && t_counter3 >= min_support
                                && !different_items[i].equals(different_items[j])
                                && !different_items[i + 1].equals(different_items[j])) {
                                product_triple_counters.put(triple_id, 1);
                            }
                        } else {
                            product_triple_counters.put(triple_id, t_counter + 1);
                        }
                    }
                }
                baskets_read++;
            }
        } catch (IOException e) {
            System.out.println("Exception 5 thrown  :" + e);
        } finally {
            try {
                br.close();
            } catch (IOException e) {
                System.out.println("Exception 6 thrown  :" + e);
            }
        }
        System.out.println(baskets_read + " baskets read");
        ask1_object.echo_results_to_file("pass3_counters.txt", product_triple_counters);
        System.out.println(product_triple_counters.size() + " different_triple_products");
        System.out.println("Pass 3 ends");
    }
        
}
