# FLAT deposit UI configuration #

=======
## General ##
When using the docker environment most settings for the deposit ui will be configured automatically. However, a few things still need to be configured manually. Please run through these steps in an ordered fashion.



## 1. Drupal file browser ##

We use the IMCE file browser to add/remove files and folders to a project. For this module the path is however not set correctly. Under [hostname]/admin/config/media/imce you need to edit profile 'User-1'. The Directory path is set using php code and should be

return 'flat_deposit/data/'.$user->name;


## 2. Enable the module ##
After building the docker image the deposit module will not be enabled automatically. You have to do this manually as local folders in which deposable data will be stored will be created for a wrong user.



## 3. Parameter change ##

```
vim /app/flat/deposit/policies/rules.sch
# adapt cmdi profiles the doorkeeper accepts by changing the 'value'-attribute in the <let>-field
# example for lat-session and MPI_Collection:
# set <let name="allowed" value="('clarin.eu:cr1:p_14077457120358', 'clarin.eu:cr1:p_1475136016239')"/>

```


### 5. Use of Owncloud ###
If you want to use owncloud, you need to enable first the sub module 'flat_owncloud'. This will creat an administrative menu in which you can activate the use of owncloud. This will give each user the option to enable owncloud which is adminstered in the users profile.



