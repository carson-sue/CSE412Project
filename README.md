# CSE412Project
Group 29

Our group is developing a bank application connected to a database. There are two types of users: bank administrators and customers. Bank administrators can view and edit a customer’s account information. A customer can view their current balance, past transactions, and withdraw and deposit money.
 
Bank accounts have a unique account ID, interest rate, current balance, and transaction log. Each bank account can only be associated with a single customer. The customers can deposit and withdraw money from a specified bank account through the command line interface.
 
Customers are associated with banking institutions. Each customer is uniquely identified by a customer ID. Customers can be associated with multiple banks and banks can have multiple customers (many-to-many relationships). Customers can access their bank account through the command line interface.
 
Banking institutions are identified by a unique ID. They can have one or more branches, each of which with an address attribute. This is a weak entity relationship, as the branch can not be uniquely identified by its address or bank manager unless it is connected to the parent banking institution.
